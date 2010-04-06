#include <jive/regalloc/shape.h>
#include <jive/regalloc/cut.h>
#include "debug.h"

/* trivial & dumb graph shaper, to test register assignment pass;
not really correct, as it does not honor regions;
also, might exceed register budget */
void
jive_regalloc_shape_old(jive_graph * graph, const jive_machine * machine,
	jive_stackframe * stack, jive_instruction_sequence * seq)
{
	jive_graphcut * cut = 0;
	
	jive_graph_traverser * trav = jive_graph_traverse_bottomup(graph);
	jive_node * current;
	
	while ( (current = jive_graph_traverse_next(trav)) != 0) {
		cut = jive_graphcut_create(graph, cut);
		
		jive_graphcut_add_instruction(cut, current, machine);
	}
	
	jive_graph_traverse_finish(trav);
	
	seq->first = cut->first;
	jive_instruction * instr = seq->first;
	while(instr->next) instr = instr->next;
	seq->last = instr;
}

typedef struct jive_shaper_context {
	/* unchanging state */
	jive_graph * graph;
	jive_stackframe * stackframe;
	const jive_machine * machine;
	
	/* registers that are active above all current cuts */
	jive_value_multiset active_above;
	jive_regcls_count active_above_count;
	
	/* currently constructed cuts */
	jive_graphcut * top, * bottom;
	
	/* auxiliary arrays that are filled temporarily */
	jive_value_multiset aux_active;
	jive_regcls_count aux_count_before, aux_count_after;
} jive_shaper_context;

static inline void
jive_shaper_context_init(jive_shaper_context * ctx, jive_graph * graph, jive_stackframe * stackframe, const jive_machine * machine)
{
	ctx->graph = graph;
	ctx->stackframe = stackframe;
	ctx->machine = machine;
	jive_value_multiset_init(&ctx->active_above);
	jive_regcls_count_init(ctx->active_above_count);
	
	ctx->top = ctx->bottom = 0;
	
	jive_value_multiset_init(&ctx->aux_active);
	jive_regcls_count_init(ctx->aux_count_before);
	jive_regcls_count_init(ctx->aux_count_after);
}

static inline bool
may_add_passthrough(
	jive_shaper_context * ctx,
	jive_instruction * new_instr)
{
	const jive_machine * machine = ctx->machine;
	jive_operand * operand = jive_node_iterate_operands((jive_node *) new_instr);
	while(operand) {
		jive_value * value = operand->value;
		if (jive_value_multiset_add(&ctx->aux_active, value) == 0) {
			jive_cpureg_class_t regcls = jive_value_get_cpureg_class(value);
			jive_regcls_count_add(ctx->aux_count_before, regcls);
			jive_regcls_count_add(ctx->aux_count_after, regcls);
		}
		operand = operand->next;
	}
	
	jive_value * value = jive_node_iterate_values((jive_node *) new_instr);
	while(value) {
		size_t count = jive_value_multiset_remove_all(&ctx->aux_active, value);
		if (count == 0) continue;
		jive_cpureg_class_t regcls = jive_value_get_cpureg_class(value);
		jive_regcls_count_sub(ctx->aux_count_before, regcls);
		jive_regcls_count_sub(ctx->aux_count_after, regcls);
		
		value = value->next;
	}
	
	if (jive_regcls_count_exceeds(ctx->aux_count_before, machine->regcls_budget)) return false;
	if (jive_regcls_count_exceeds(ctx->aux_count_after, machine->regcls_budget)) return false;
	
	return true;
}

static inline bool
may_add_below(
	jive_shaper_context * ctx,
	jive_instruction * instr,
	jive_instruction * new_instr)
{
	jive_output_edge_iterator it;
	JIVE_ITERATE_OUTPUTS(it, (jive_node *)new_instr)
		if (it->target.node == (jive_node *) instr) return false;
	
	jive_value_multiset_copy(&ctx->aux_active, &instr->active_before);
	jive_regcls_count_copy(ctx->aux_count_before, instr->use_count_before);
	jive_regcls_count_copy(ctx->aux_count_after, instr->use_count_after);
	
	return may_add_passthrough(ctx, new_instr);
}

static inline bool
may_add_before(
	jive_shaper_context * ctx,
	jive_instruction * instr,
	jive_instruction * new_instr)
{
	const jive_machine * machine = ctx->machine;
	jive_value_multiset_copy(&ctx->aux_active, &instr->active_before);
	jive_regcls_count_copy(ctx->aux_count_before, instr->use_count_before);
	jive_regcls_count_copy(ctx->aux_count_after, instr->use_count_before);
	
	jive_value * value = jive_node_iterate_values((jive_node *) new_instr);
	while(value) {
		size_t count = jive_value_multiset_remove_all(&ctx->aux_active, value);
		jive_cpureg_class_t regcls = jive_value_get_cpureg_class(value);
		if (count == 0) {
			jive_regcls_count_add(ctx->aux_count_after, regcls);
		} else {
			jive_regcls_count_sub(ctx->aux_count_before, regcls);
		}
		
		value = value->next;
	}
	
	jive_operand * operand = jive_node_iterate_operands((jive_node *) new_instr);
	while(operand) {
		jive_value * value = operand->value;
		if (jive_value_multiset_add(&ctx->aux_active, value) == 0) {
			jive_cpureg_class_t regcls = jive_value_get_cpureg_class(value);
			jive_regcls_count_add(ctx->aux_count_before, regcls);
		}
		operand = operand->next;
	}
	
	if (jive_regcls_count_exceeds(ctx->aux_count_before, machine->regcls_budget)) return false;
	if (jive_regcls_count_exceeds(ctx->aux_count_after, machine->regcls_budget)) return false;
	
	return true;
}

static inline void
do_add_before(
	jive_shaper_context * ctx,
	jive_instruction * instr,
	jive_instruction * new_instr)
{
	jive_instruction * tmp = instr;
	if (tmp) tmp = tmp->prev;
	while(tmp) {
		tmp = tmp->prev;
		bool may_add = may_add_below(ctx, tmp, new_instr);
		DEBUG_ASSERT(may_add);
		(void)may_add;
		jive_value_multiset_copy(&tmp->active_before, &ctx->aux_active);
		jive_regcls_count_copy(tmp->use_count_before, ctx->aux_count_before);
		jive_regcls_count_copy(tmp->use_count_after, ctx->aux_count_after);
		
		tmp = tmp->prev;
	}
	
	bool may_add = may_add_before(ctx, instr, new_instr);
	DEBUG_ASSERT(may_add);
	(void)may_add;
	jive_value_multiset_copy(&tmp->active_before, &ctx->aux_active);
	jive_regcls_count_copy(tmp->use_count_before, ctx->aux_count_before);
	jive_regcls_count_copy(tmp->use_count_after, ctx->aux_count_after);
}

static inline void
shape_instruction(jive_shaper_context * ctx, jive_instruction * new_instr)
{
	/* check whether input regs can be added */
	jive_value_multiset_copy(&ctx->aux_active, &ctx->active_above);
	jive_regcls_count_copy(ctx->aux_count_before, ctx->active_above_count);
	jive_regcls_count_copy(ctx->aux_count_after, ctx->active_above_count);
	
	DEBUG_ASSERT(may_add_passthrough(ctx, new_instr));
	
	/* find lowest cut at the end of which this instruction may be added */
	jive_graphcut * lowest_cut = 0, * cut = ctx->top;
	while(cut) {
		jive_instruction * instr = cut->first;
		while(true) {
			if (!may_add_below(ctx, instr, new_instr)) break;
			if (instr == cut->last) {
				instr = 0;
				break;
			}
			instr = instr->next;
		}
		if (instr) break;
		if (may_add_before(ctx, cut->lower->first, new_instr))
			lowest_cut = cut->upper;
		cut = cut->lower;
	}
	
	if (!lowest_cut) {
		lowest_cut = jive_graphcut_create(ctx->graph, ctx->top);
		ctx->top = lowest_cut;
		if (!ctx->bottom) ctx->bottom = lowest_cut;
	}
	
	jive_graphcut_add_instruction(lowest_cut, (jive_node *)new_instr, ctx->machine);
	
	/* update register states of all instructions above */
	jive_instruction * instr = ctx->top->first;
	while(instr != new_instr) {
		may_add_below(ctx, instr, new_instr);
		jive_value_multiset_copy(&instr->active_before, &ctx->aux_active);
		jive_regcls_count_copy(instr->use_count_before, ctx->aux_count_before);
		jive_regcls_count_copy(instr->use_count_after, ctx->aux_count_after);
	
		
		instr = instr->next;
	}
	
	/* update register state above top-most cut */
	jive_value_multiset_copy(&ctx->aux_active, &ctx->active_above);
	jive_regcls_count_copy(ctx->aux_count_before, ctx->active_above_count);
	jive_regcls_count_copy(ctx->aux_count_after, ctx->active_above_count);
	
	may_add_passthrough(ctx, new_instr);
	jive_value_multiset_copy(&ctx->active_above, &ctx->aux_active);
	jive_regcls_count_copy(ctx->active_above_count, ctx->aux_count_before);
}

void
jive_regalloc_shape(jive_graph * graph, const jive_machine * machine,
	jive_stackframe * stack, jive_instruction_sequence * seq)
{
	jive_shaper_context ctx;
	jive_shaper_context_init(&ctx, graph, stack, machine);
	
	jive_graphcut * cut = 0;
	
	jive_value_multiset active_before;
	jive_value_multiset_init(&active_before);
	
	jive_graph_traverser * trav = jive_graph_traverse_bottomup(graph);
	jive_node * current;
	
	while ( (current = jive_graph_traverse_next(trav)) != 0) {
		cut = jive_graphcut_create(graph, cut);
		
		jive_graphcut_add_instruction(cut, current, machine);
	}
	
	jive_graph_traverse_finish(trav);
	
	seq->first = cut->first;
	jive_instruction * instr = seq->first;
	while(instr->next) instr = instr->next;
	seq->last = instr;
}
