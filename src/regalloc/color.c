#include <jive/regalloc/color.h>
#include <jive/vsdg/valuetype-private.h>
#include <jive/vsdg/graph-private.h>
#include <jive/vsdg/resource-interference-private.h>
#include <jive/vsdg/regcls-count-private.h>
#include <jive/vsdg/crossings-private.h>
#include <jive/vsdg/traverser.h>
#include <jive/debug-private.h>
#include <jive/arch/instruction.h>

static bool
can_specialize(jive_value_resource * regcand, const jive_regcls * regcls)
{
	const jive_regcls * old_regcls = jive_resource_get_regcls(&regcand->base);
	const jive_regcls * new_regcls = jive_regcls_intersection(regcls, old_regcls);
	if (!new_regcls || (old_regcls == new_regcls)) return false;
	const jive_regcls * overflow = jive_value_resource_check_change_regcls(regcand, new_regcls);
	return overflow == 0;
}

static bool
try_specialize(jive_value_resource * regcand, const jive_regcls * regcls)
{
	const jive_regcls * old_regcls = jive_resource_get_regcls(&regcand->base);
	const jive_regcls * new_regcls = jive_regcls_intersection(regcls, old_regcls);
	if (!new_regcls || (old_regcls == new_regcls)) return false;
	const jive_regcls * overflow = jive_value_resource_check_change_regcls(regcand, new_regcls);
	if (overflow) return false;
	
	jive_value_resource_set_regcls(regcand, new_regcls);
	return true;
}

static void
hard_specialize_recursive(jive_node * node)
{
	if (!jive_node_isinstance(node, &JIVE_INSTRUCTION_NODE)) return;
	const jive_instruction_class * icls = ((jive_instruction_node *)node)->icls;
	
	if (!(icls->flags & jive_instruction_write_input)) return;
	if (icls->flags & jive_instruction_commutative) {
		const jive_regcls * in0_regcls = jive_resource_get_regcls(node->inputs[0]->resource);
		const jive_regcls * in1_regcls = jive_resource_get_regcls(node->inputs[1]->resource);
		const jive_regcls * out_regcls = jive_resource_get_regcls(node->outputs[0]->resource);
		if ((in0_regcls == out_regcls) || (in1_regcls == out_regcls)) return;
		if (out_regcls->nregs == 1) {
			bool can0 = can_specialize((jive_value_resource *) node->inputs[0]->resource, out_regcls);
			bool can1 = can_specialize((jive_value_resource *) node->inputs[1]->resource, out_regcls);
			if ((can0 && can1) || (!can0 && !can1)) return;
			
			jive_input * input;
			if (can0) input = node->inputs[0];
			else input = node->inputs[1];
			if (try_specialize((jive_value_resource *) input->resource, out_regcls))
				hard_specialize_recursive(input->origin->node);
		} else if ((in0_regcls->nregs == 1) && (in1_regcls->nregs == 1)) {
			bool can0 = can_specialize((jive_value_resource *) node->outputs[0]->resource, in0_regcls);
			bool can1 = can_specialize((jive_value_resource *) node->outputs[0]->resource, in1_regcls);
			if ((can0 && can1) || (!can0 && !can1)) return;
			
			const jive_regcls * regcls;
			if (can0) regcls = in0_regcls;
			else regcls = in1_regcls;
			if (try_specialize((jive_value_resource *) node->outputs[0]->resource, regcls)) {
				jive_input * user;
				JIVE_LIST_ITERATE(node->outputs[0]->users, user, output_users_list)
					hard_specialize_recursive(user->node);
			}
		}
	} else {
		const jive_regcls * in_regcls = jive_resource_get_regcls(node->inputs[0]->resource);
		const jive_regcls * out_regcls = jive_resource_get_regcls(node->outputs[0]->resource);
		if (in_regcls == out_regcls) return;
		if (in_regcls->nregs == 1) {
			if (try_specialize((jive_value_resource *) node->outputs[0]->resource, in_regcls)) {
				jive_input * user;
				JIVE_LIST_ITERATE(node->outputs[0]->users, user, output_users_list)
					hard_specialize_recursive(user->node);
			}
		} else if (out_regcls->nregs == 1) {
			if (try_specialize((jive_value_resource *) node->inputs[0]->resource, out_regcls))
				hard_specialize_recursive(node->inputs[0]->origin->node);
		}
	}
}

static bool
soft_specialize_recursive(jive_node * node)
{
	if (!jive_node_isinstance(node, &JIVE_INSTRUCTION_NODE)) return false;
	const jive_instruction_class * icls = ((jive_instruction_node *)node)->icls;
	
	if (!(icls->flags & jive_instruction_write_input)) return false;
	
	if (try_specialize((jive_value_resource *) node->inputs[0]->resource, jive_resource_get_regcls(node->outputs[0]->resource))) {
		hard_specialize_recursive(node->inputs[0]->origin->node);
		return true;
	}
	if (try_specialize((jive_value_resource *) node->outputs[0]->resource, jive_resource_get_regcls(node->inputs[0]->resource))) {
		jive_input * user;
		JIVE_LIST_ITERATE(node->outputs[0]->users, user, output_users_list)
			hard_specialize_recursive(user->node);
		return true;
	}
	
	if (!(icls->flags & jive_instruction_write_input)) return false;
	
	if (try_specialize((jive_value_resource *) node->inputs[1]->resource, jive_resource_get_regcls(node->outputs[0]->resource))) {
		hard_specialize_recursive(node->inputs[1]->origin->node);
		return true;
	}
	if (try_specialize((jive_value_resource *) node->outputs[0]->resource, jive_resource_get_regcls(node->inputs[1]->resource))) {
		jive_input * user;
		JIVE_LIST_ITERATE(node->outputs[0]->users, user, output_users_list)
			hard_specialize_recursive(user->node);
		return true;
	}
	
	return false;
}

static void
pre_specialize(jive_graph * graph)
{
	/* try to specialize the register class of selected register candidates
	to ensure that input/output registers of instructions that overwrite
	one of their inputs match (to avoid inserting instructions later
	during fixup) */
	
	jive_node * node;
	
	/* first, apply all "inevitable" specialization (i.e. those were
	there is no choice between two alternative registers) */
	jive_traverser * traverser = jive_bottomup_traverser_create(graph);
	while( (node = jive_traverser_next(traverser)) != 0) hard_specialize_recursive(node);
	jive_traverser_destroy(traverser);
	
	/* now apply useful specializations where we can choose
	between two possible inputs that could be overwritten
	(commutative operations) */
	for(;;) {
		bool progress = false;
		jive_traverser * traverser = jive_bottomup_traverser_create(graph);
		while( (node = jive_traverser_next(traverser)) != 0)
			if (soft_specialize_recursive(node)) progress = true;
		jive_traverser_destroy(traverser);
		if (!progress) break;
	}
}

static void
update_max_use_count(jive_regcls_count * dst, jive_regcls_count * src, jive_context * context)
{
	jive_regcls_count_iterator i;
	for(i = jive_regcls_count_begin(src); i.entry; jive_regcls_count_iterator_next(&i)) {
		jive_regcls_count_max(dst, context, i.entry->regcls, i.entry->count);
	}
}

static void
compute_max_cross_count(jive_resource * resource, jive_regcls_count * use_count, jive_context * context)
{
	jive_node_resource_interaction * xpoint;
	JIVE_LIST_ITERATE(resource->node_interaction, xpoint, same_resource_list) {
		if (xpoint->before_count)
			update_max_use_count(use_count, &xpoint->node->use_count_before, context);
		if (xpoint->after_count)
			update_max_use_count(use_count, &xpoint->node->use_count_after, context);
	}
}

static const jive_cpureg *
find_allowed_register(jive_value_resource * regcand)
{
	jive_context * context = regcand->base.graph->context;
	const jive_cpureg * best_reg = 0;
	const jive_regcls * regcls = regcand->regcls;
	size_t best_pressure = regcand->base.interference.nitems + 1;
	
	jive_regcls_count use_count;
	jive_regcls_count_init(&use_count);
	compute_max_cross_count(&regcand->base, &use_count, context);
	
	struct jive_allowed_registers_hash_iterator i;
	JIVE_HASH_ITERATE(jive_allowed_registers_hash, regcand->allowed_registers, i) {
		const jive_cpureg * reg = i.entry->reg;
		
		if (jive_regcls_count_check_change(&use_count, regcls, reg->regcls))
			continue;
		
		size_t pressure = 0;
		
		/* count the neighbours who could legally also be assigned this register */
		struct jive_resource_interference_hash_iterator j;
		JIVE_HASH_ITERATE(jive_resource_interference_hash, regcand->base.interference, j) {
			jive_resource * resource = j.entry->resource;
			if (!jive_resource_isinstance(resource, &JIVE_VALUE_RESOURCE)) continue;
			
			jive_value_resource * vresource = (jive_value_resource *) resource;
			if (jive_allowed_registers_hash_lookup(&vresource->allowed_registers, reg))
				pressure ++;
		}
		
		/* pick the one that is least constraining for the neighbours */
		if (pressure < best_pressure) {
			best_pressure = pressure;
			best_reg = reg;
		}
	}
	
	jive_regcls_count_fini(&use_count, context);
	
	DEBUG_ASSERT(best_reg);
	
	return best_reg;
}

static void
color_single(jive_graph * graph, jive_value_resource * regcand)
{
	const jive_cpureg * reg = find_allowed_register(regcand);
	/* TODO: implement conflict resolution */
	DEBUG_ASSERT(reg);
	jive_value_resource_set_cpureg(regcand, reg);
	
	jive_input * input;
	JIVE_LIST_ITERATE(regcand->base.inputs, input, resource_input_list)
		hard_specialize_recursive(input->node);
	
	jive_output * output;
	JIVE_LIST_ITERATE(regcand->base.outputs, output, resource_output_list)
		hard_specialize_recursive(output->node);
}

static jive_value_resource *
find_next_uncolored(jive_graph * graph)
{
	if (graph->valueres.max_pressure)
		return graph->valueres.pressured[graph->valueres.max_pressure - 1].first;
	if (graph->valueres.trivial.first)
		return graph->valueres.trivial.first;
	return 0;
}

void
jive_regalloc_color(jive_graph * graph)
{
	pre_specialize(graph);
	for(;;) {
		jive_value_resource * regcand = find_next_uncolored(graph);
		if (!regcand) return;
		if (!jive_resource_used(&regcand->base)) {
			jive_resource_destroy(&regcand->base);
			continue;
		}
		color_single(graph, regcand);
	}
}
