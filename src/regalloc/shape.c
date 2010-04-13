#include <jive/regalloc/shape.h>
#include <jive/regalloc/cut.h>
#include <jive/regalloc/spill.h>
#include "debug.h"
#include <jive/graphdebug.h>

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
	
	/* instructions eligible for placement next */
	jive_instruction * avail_first, * avail_last;
	
	/* auxiliary arrays that are filled temporarily */
	jive_value_multiset aux_active;
	jive_regcls_count aux_count_before, aux_count_after;
} jive_shaper_context;

static void
jive_shaper_verify_state(jive_shaper_context * ctx)
{
	jive_instruction * instr = 0;
	if (ctx->top) instr = ctx->top->first;
	while(instr) {
		DEBUG_ASSERT(instr->cut && instr->cut != (jive_graphcut *)-1);
		jive_output_edge_iterator it;
		JIVE_ITERATE_OUTPUTS(it, (jive_node *) instr) {
			jive_instruction * tmp = (jive_instruction *)it->target.node;
			DEBUG_ASSERT(tmp->cut && tmp->cut != (jive_graphcut *)-1);
		}
		
		instr = instr->next;
	}
}

/* push instruction to front of list of available instructions */
static inline void
jive_shaper_context_add_available_instruction(jive_shaper_context * ctx, jive_instruction * instr)
{
	DEBUG_ASSERT(instr->cut == 0 || instr->cut == (jive_graphcut *)-1);
	if (instr->cut) return;
	
	DEBUG_PRINTF("add_available: %p\n", instr);
	instr->prev = 0;
	instr->next = ctx->avail_first;
	if (ctx->avail_first) ctx->avail_first->prev = instr;
	else ctx->avail_last = instr;
	ctx->avail_first = instr;
	
	instr->cut = (jive_graphcut *) -1;
	
#ifdef DEBUG
	jive_output_edge_iterator it;
	JIVE_ITERATE_OUTPUTS(it, (jive_node *) instr) {
		jive_instruction * tmp = (jive_instruction *) it->target.node;
		DEBUG_ASSERT(tmp->cut && tmp->cut != (jive_graphcut *)-1);
	}
#endif
}

static inline void
jive_shaper_context_add_available_instruction_back(jive_shaper_context * ctx, jive_instruction * instr)
{
	DEBUG_ASSERT(instr->cut == 0 || instr->cut == (jive_graphcut *)-1);
	if (instr->cut) return;
	
	DEBUG_PRINTF("add_available_back: %p\n", instr);
	instr->prev = ctx->avail_last;
	instr->next = 0;
	if (ctx->avail_last) ctx->avail_last->next = instr;
	else ctx->avail_first = instr;
	ctx->avail_last = instr;
	
	instr->cut = (jive_graphcut *) -1;
	
#ifdef DEBUG
	jive_output_edge_iterator it;
	JIVE_ITERATE_OUTPUTS(it, (jive_node *) instr) {
		jive_instruction * tmp = (jive_instruction *) it->target.node;
		DEBUG_ASSERT(tmp->cut && tmp->cut != (jive_graphcut *)-1);
	}
#endif
}

static inline void
jive_shaper_context_remove_available_instruction(jive_shaper_context * ctx, jive_instruction * instr)
{
	if (instr->cut == 0) return;
	
	DEBUG_PRINTF("remove_available: %p\n", instr);
	
	if (instr->prev) instr->prev->next = instr->next;
	else ctx->avail_first = instr->next;
	if (instr->next) instr->next->prev = instr->prev;
	else ctx->avail_last = instr->prev;
	
	instr->prev = instr->next = 0;
	instr->cut = 0;
}

static inline void
jive_shaper_context_try_add_available_instruction(jive_shaper_context * ctx, jive_instruction * instr)
{
	if (instr->cut) return;
	
	jive_output_edge_iterator it;
	JIVE_ITERATE_OUTPUTS(it, (jive_node *) instr) {
		jive_instruction * tmp = (jive_instruction *) it->target.node;
		if (!tmp->cut || tmp->cut == (jive_graphcut *)-1)
			return;
	}
	
	jive_shaper_context_add_available_instruction(ctx, instr);
}

static inline void
jive_shaper_context_check_dependents(jive_shaper_context * ctx, jive_instruction * instr)
{
	jive_input_edge_iterator it;
	JIVE_ITERATE_INPUTS(it, (jive_node *) instr) {
		jive_instruction * tmp = (jive_instruction *) it->origin.node;
		jive_shaper_context_try_add_available_instruction(ctx, tmp);
	}
}

static inline jive_instruction *
jive_shaper_context_get_available_instruction(jive_shaper_context * ctx)
{
	jive_instruction * tmp = ctx->avail_first;
	while(tmp) {DEBUG_PRINTF("%p ", tmp); tmp=tmp->next;}
	DEBUG_PRINTF("\n");
	jive_instruction * instr = ctx->avail_first;
	if (instr->next) instr->next->prev = 0;
	else ctx->avail_last = 0;
	ctx->avail_first = instr->next;
	instr->prev = instr->next = 0;
	
	DEBUG_ASSERT(instr->cut = (jive_graphcut *)-1);
	
	return instr;
}

static inline void
jive_shaper_context_init(jive_shaper_context * ctx, jive_graph * graph, jive_stackframe * stackframe, const jive_machine * machine)
{
	ctx->graph = graph;
	ctx->stackframe = stackframe;
	ctx->machine = machine;
	jive_value_multiset_init(&ctx->active_above);
	jive_regcls_count_init(ctx->active_above_count);
	
	ctx->top = ctx->bottom = 0;
	
	ctx->avail_first = ctx->avail_last = 0;
	
	jive_value_multiset_init(&ctx->aux_active);
	jive_regcls_count_init(ctx->aux_count_before);
	jive_regcls_count_init(ctx->aux_count_after);
	
	jive_input_edge_iterator it;
	JIVE_ITERATE_BOTTOM(it, graph) {
		jive_instruction * instr = (jive_instruction *) it->origin.node;
		jive_shaper_context_add_available_instruction(ctx, instr);
	}
}

static jive_cpureg_class_t
overflows_passthrough(
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
		if (count != 0) {
			jive_cpureg_class_t regcls = jive_value_get_cpureg_class(value);
			jive_regcls_count_sub(ctx->aux_count_before, regcls);
			jive_regcls_count_sub(ctx->aux_count_after, regcls);
		}
		
		value = value->next;
	}
	
	jive_cpureg_class_t excess;
	
	excess = jive_regcls_count_exceeds_class(ctx->aux_count_before, machine);
	if (excess) return excess;
	excess = jive_regcls_count_exceeds_class(ctx->aux_count_after, machine);
	if (excess) return excess;
	
	return 0;
}

static bool
must_sequence_before(jive_instruction * instr, jive_instruction * new_instr)
{
	jive_output_edge_iterator it;
	JIVE_ITERATE_OUTPUTS(it, (jive_node *)new_instr)
		if (it->target.node == (jive_node *) instr) return true;
	return false;
	
}

static jive_cpureg_class_t
overflows_below(
	jive_shaper_context * ctx,
	jive_instruction * instr,
	jive_instruction * new_instr)
{
	jive_value_multiset_copy(&ctx->aux_active, &instr->active_before);
	jive_regcls_count_copy(ctx->aux_count_before, instr->use_count_before);
	jive_regcls_count_copy(ctx->aux_count_after, instr->use_count_after);
	
	return overflows_passthrough(ctx, new_instr);
}

static jive_cpureg_class_t
overflows_before(
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
	
	jive_cpureg_class_t excess;
	excess = jive_regcls_count_exceeds_class(ctx->aux_count_before, machine);
	if (excess) return excess;
	excess = jive_regcls_count_exceeds_class(ctx->aux_count_after, machine);
	if (excess) return excess;
	
	return 0;
}

static jive_cpureg_class_t
shape_instruction(jive_shaper_context * ctx, jive_instruction * new_instr)
{
	/* check whether input regs can be added */
	jive_value_multiset_copy(&ctx->aux_active, &ctx->active_above);
	jive_regcls_count_copy(ctx->aux_count_before, ctx->active_above_count);
	jive_regcls_count_copy(ctx->aux_count_after, ctx->active_above_count);
	
	jive_cpureg_class_t overflow = overflows_passthrough(ctx, new_instr);
	if (overflow) return overflow;
	
	/* FIXME: still missing: compute "shared" register classes */
	
	/* find lowest cut at the end of which this instruction may be added */
	jive_graphcut * lowest_cut = 0, * cut = ctx->top;
	while(cut) {
		jive_instruction * instr = cut->first;
		while(true) {
			if (must_sequence_before(instr, new_instr)) break;
			if (overflows_below(ctx, instr, new_instr)) break;
			if (instr == cut->last) {
				instr = 0;
				break;
			}
			instr = instr->next;
		}
		if (instr) break;
		if (overflows_before(ctx, cut->lower->first, new_instr)==0)
			lowest_cut = cut;
		cut = cut->lower;
	}
	
	if (lowest_cut == 0) {
		lowest_cut = jive_graphcut_create(ctx->graph, ctx->top);
		ctx->top = lowest_cut;
		if (!ctx->bottom) ctx->bottom = lowest_cut;
	}
	
	if (lowest_cut->lower)
		DEBUG_ASSERT(overflows_before(ctx, lowest_cut->lower->first, new_instr)==0);
	jive_graphcut_add_instruction(lowest_cut, (jive_node *)new_instr, ctx->machine);
	
	/* update register states of all instructions above */
	jive_instruction * instr = ctx->top->first;
	while(instr != new_instr) {
		overflows_below(ctx, instr, new_instr);
		jive_value_multiset_copy(&instr->active_before, &ctx->aux_active);
		jive_regcls_count_copy(instr->use_count_before, ctx->aux_count_before);
		jive_regcls_count_copy(instr->use_count_after, ctx->aux_count_after);
		
		instr = instr->next;
	}
	
	/* update register state above top-most cut */
	jive_value_multiset_copy(&ctx->aux_active, &ctx->active_above);
	jive_regcls_count_copy(ctx->aux_count_before, ctx->active_above_count);
	jive_regcls_count_copy(ctx->aux_count_after, ctx->active_above_count);
	
	overflows_passthrough(ctx, new_instr);
	jive_value_multiset_copy(&ctx->active_above, &ctx->aux_active);
	jive_regcls_count_copy(ctx->active_above_count, ctx->aux_count_before);
	
	jive_shaper_verify_state(ctx);
	
	return 0;
}

/* FIXME: also declared in assign.c */
static bool
jive_instruction_uses(jive_instruction * instr, jive_value * value)
{
	jive_operand * operand = jive_node_iterate_operands((jive_node *) instr);
	while(operand) {
		if (operand->value == value) return true;
		operand = operand->next;
	}
	return false;
}

static inline bool
jive_instruction_defines(jive_instruction * instr, jive_value * value)
{
	return value->node == (jive_node *) instr;
}

static void
spill_for(jive_shaper_context * ctx, jive_instruction * instr, jive_cpureg_class_t contended_class)
{
	/* pick value to be spilled */
	size_t n;
	jive_value * spilled_value = 0;
	for(n=0; n<ctx->active_above.nitems; n++) {
		jive_value * value = ctx->active_above.items[n].value;
		jive_cpureg_class_t regcls = jive_value_get_cpureg_class_shared(value);
		
		if (!jive_cpureg_class_contains(contended_class, regcls))
			continue;
		if (jive_instruction_uses(instr, value))
			continue;
		if (jive_instruction_defines(instr, value))
			continue;
		
		spilled_value = value;
		break;
	}
	
	DEBUG_ASSERT(spilled_value);
	
	jive_instruction * spill = (jive_instruction *) jive_regalloc_spill(spilled_value, ctx->machine, ctx->stackframe);
	
	jive_value * restored_value = jive_regalloc_restore((jive_node *)spill, ctx->machine, ctx->stackframe);
	jive_state_edge_create((jive_node *) spill, (jive_node *) restored_value->node);
	jive_cpureg_class_t regcls = jive_value_get_cpureg_class_shared(spilled_value);
	jive_value_set_cpureg_class_shared(restored_value, regcls);
	
	/* divert instructions to use restored value */
	jive_output_edge_iterator i = jive_node_iterate_outputs(spilled_value->node);
	while(i) {
		jive_edge * e = i;
		i = jive_output_edge_iterator_next(i);
		if (e->origin.port != (jive_value *) spilled_value) continue;
		
		jive_instruction * user = (jive_instruction *)e->target.node;
		if (user->cut && user->cut != (jive_graphcut *)-1) {
			jive_edge_divert_origin(e, restored_value);
		}
	}
	
	jive_instruction * definer = (jive_instruction *) spilled_value->node;
	DEBUG_ASSERT(definer->cut == 0 || definer->cut == (jive_graphcut *) -1);
	jive_shaper_context_remove_available_instruction(ctx, definer);
	
	/* replace occurence of spilled_value in active sets */
	jive_instruction * tmp = ctx->top->first;
	while(tmp) {
		if (jive_value_multiset_contains(&tmp->active_before, spilled_value)) {
			jive_value_multiset_remove_all(&tmp->active_before, spilled_value);
			jive_value_multiset_add(&tmp->active_before, restored_value);
		}
		tmp = tmp->next;
	}
	
	jive_value_multiset_remove_all(&ctx->active_above, spilled_value);
	jive_value_multiset_add(&ctx->active_above, restored_value);
	
	/* force issuing of restore instruction */
	shape_instruction(ctx, (jive_instruction *) restored_value->node);
	
	jive_shaper_context_add_available_instruction_back(ctx, spill);
	
	DEBUG_ASSERT(spill->cut == (jive_graphcut *) -1);
}

void
jive_regalloc_shape(jive_graph * graph, const jive_machine * machine,
	jive_stackframe * stack, jive_instruction_sequence * seq)
{
	jive_shaper_context ctx;
	jive_shaper_context_init(&ctx, graph, stack, machine);
	
	jive_value_multiset active_before;
	jive_value_multiset_init(&active_before);
	
	while(ctx.avail_first) {
		jive_instruction * instr = jive_shaper_context_get_available_instruction(&ctx);
		for(;;) {
			jive_cpureg_class_t contended_class;
			contended_class = shape_instruction(&ctx, instr);
			if (!contended_class) break;
			spill_for(&ctx, instr, contended_class);
		}
		jive_shaper_context_check_dependents(&ctx, instr);
	}
	
	seq->first = ctx.top->first;
	jive_instruction * instr = seq->first;
	while(instr->next) instr = instr->next;
	seq->last = instr;
}

