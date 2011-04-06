#include <jive/regalloc/color.h>

#include <jive/regalloc/assignment-tracker-private.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/shaped-variable-private.h>

static jive_shaped_variable *
find_next_uncolored(const jive_var_assignment_tracker * tracker)
{
	if (tracker->pressured_max) {
		JIVE_DEBUG_ASSERT(tracker->pressured[tracker->pressured_max - 1].first);
		return tracker->pressured[tracker->pressured_max - 1].first;
	} else if (tracker->trivial.first) {
		return tracker->trivial.first;
	} else return NULL;
}

#include <stdio.h>

void
jive_regalloc_color(jive_shaped_graph * shaped_graph)
{
	for(;;) {
		jive_shaped_variable * shaped_variable = find_next_uncolored(&shaped_graph->var_assignment_tracker);
		if (!shaped_variable)
			break;
		
		printf("%p\n", shaped_variable);
		
		const jive_resource_name * name = 0;
		struct jive_allowed_resource_names_hash_iterator i;
		JIVE_HASH_ITERATE(jive_allowed_resource_names_hash, shaped_variable->allowed_names, i) {
			name = i.entry->name;
			break;
		}
		
		jive_variable_set_resource_name(shaped_variable->variable, name);
	}
}

#if 0

#include <jive/regalloc/color.h>
#include <jive/regalloc/auxnodes.h>
#include <jive/vsdg/valuetype-private.h>
#include <jive/vsdg/graph-private.h>
#include <jive/vsdg/resource-interference-private.h>
#include <jive/vsdg/regcls-count-private.h>
#include <jive/vsdg/crossings-private.h>
#include <jive/vsdg/traverser.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/cut.h>
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
	const jive_instruction_class * icls = ((jive_instruction_node *)node)->attrs.icls;
	
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
	const jive_instruction_class * icls = ((jive_instruction_node *)node)->attrs.icls;
	
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
	
	return best_reg;
}

typedef struct jive_region_split jive_region_split;

struct jive_region_split {
	jive_region * region;
	jive_node_location * point;
	struct {
		size_t nitems, space;
		jive_input ** items;
	} users;
	struct {
		jive_region_split * prev;
		jive_region_split * next;
	} split_list;
};

static jive_region_split *
jive_region_split_create(jive_region * region)
{
	jive_region_split * split = jive_context_malloc(region->graph->context, sizeof(*split));
	split->region = region;
	split->point = 0;
	split->users.nitems = split->users.space = 0;
	split->users.items = 0;
	return split;
}

static void
jive_region_split_destroy(jive_region_split * self)
{
	jive_context_free(self->region->graph->context, self->users.items);
	jive_context_free(self->region->graph->context, self);
}

static void
jive_region_split_add_user(jive_region_split * self, jive_input * user)
{
	if (self->users.nitems >= self->users.space) {
		self->users.space = self->users.nitems * 2 + 1;
		self->users.items = jive_context_realloc(self->region->graph->context, self->users.items,
			sizeof(self->users.items[0]) * self->users.space);
	}
	self->users.items[self->users.nitems ++] = user;
}

static const jive_cpureg *
simple_splitting(jive_value_resource * regcand)
{
	jive_output * value = regcand->base.outputs.first;
	DEBUG_ASSERT(regcand->base.outputs.last == regcand->base.outputs.first);
	
	const jive_regcls * regcls = jive_resource_get_regcls(&regcand->base);
	
	jive_node * spill_node = jive_aux_spill_node_create(value->node->region, regcls, value);
	jive_cut_insert(value->node->shape_location->cut, value->node->shape_location->cut_nodes_list.next, spill_node);
	jive_resource_assign_input(&regcand->base, spill_node->inputs[0]);
	jive_output * stackslot = spill_node->outputs[0];
	jive_resource_assign_output(jive_output_get_constraint(stackslot), stackslot);
	
	struct {
		jive_region_split * first;
		jive_region_split * last;
	} splits = {0,0};
	
	const jive_cpureg * reg = 0;
	while(!reg) {
		jive_input * last_user = 0, * input;
		JIVE_LIST_ITERATE(regcand->base.inputs, input, resource_input_list) {
			const jive_node_resource_interaction * xpoint;
			xpoint = jive_node_resource_interaction_lookup(input->node, &regcand->base);
			if (xpoint->crossed_count == 0) {
				last_user = input;
				break;
			}
		}
		
		DEBUG_ASSERT(last_user);
		
		jive_region_split * split;
		JIVE_LIST_ITERATE(splits, split, split_list) if (split->region == last_user->node->region) break;
		if (!split) {
			split = jive_region_split_create(last_user->node->region);
			JIVE_LIST_PUSH_FRONT(splits, split, split_list);
		}
		split->point = last_user->node->shape_location;
		jive_region_split_add_user(split, last_user);
		/* if split is above sub-regions, users within the sub-regions
		can use the restored value in their parent region and do
		not need a separate split */
		jive_region_split * other_split, * next_split;
		JIVE_LIST_ITERATE_SAFE(splits, other_split, next_split, split_list) {
			if (jive_region_is_contained_by(other_split->region, split->region)) {
				size_t n;
				for(n=0; n<other_split->users.nitems; n++)
					jive_region_split_add_user(split, other_split->users.items[n]);
				JIVE_LIST_REMOVE(splits, other_split, split_list);
				jive_region_split_destroy(other_split);
			}
		}
		
		jive_resource_unassign_input(&regcand->base, last_user);
		jive_value_resource_recompute_regcls(regcand);
		reg = find_allowed_register(regcand);
	}
	
	jive_region_split * split, * next_split;
	JIVE_LIST_ITERATE_SAFE(splits, split, next_split, split_list) {
		jive_node * restore_node = jive_aux_restore_node_create(split->region, regcls, stackslot);
		jive_resource_assign_input(stackslot->resource, restore_node->inputs[0]);
		jive_output * restored_value = restore_node->outputs[0];
		jive_resource_assign_output(jive_output_get_constraint(restored_value), restored_value);
		jive_resource * res = restored_value->resource;
		size_t n;
		for(n=0; n<split->users.nitems; n++) {
			jive_input * user = split->users.items[n];
			jive_input_divert_origin(user, restored_value);
			jive_resource_assign_input(res, user);
		}
		jive_value_resource_recompute_regcls((jive_value_resource *) res);
		jive_cut_insert(split->point->cut, split->point, restore_node);
		jive_region_split_destroy(split);
	}
	
	return reg;
}

static void
color_single(jive_graph * graph, jive_value_resource * regcand)
{
	const jive_cpureg * reg = find_allowed_register(regcand);
	if (!reg) {
		reg = simple_splitting(regcand);
		/* TODO: implement conflict resolution for gated values */
	}
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

#endif