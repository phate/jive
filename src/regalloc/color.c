#include <jive/regalloc/color.h>

#include <jive/regalloc/assignment-tracker-private.h>
#include <jive/regalloc/auxnodes.h>
#include <jive/regalloc/crossing-arc.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/shaped-variable-private.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/region.h>

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

static const jive_resource_name *
find_allowed_name(jive_context * context, jive_shaped_variable * candidate)
{
	jive_variable * variable = candidate->variable;
	
	/* of the allowed registers, choose the one that puts the
	smallest amount of "pressure" on interfering variables */
	
	jive_resource_class_count cross_count;
	jive_resource_class_count_init(&cross_count, context);
	jive_shaped_variable_get_cross_count(candidate, &cross_count);
	
	const jive_resource_class * rescls = jive_variable_get_resource_class(variable);
	
	const jive_resource_name * best_name = 0;
	size_t best_pressure = candidate->interference.nitems + 1;
	
	struct jive_allowed_resource_names_hash_iterator i;
	JIVE_HASH_ITERATE(jive_allowed_resource_names_hash, candidate->allowed_names, i) {
		const jive_resource_name * name = i.entry->name;
		size_t pressure = 0;
		
		const jive_resource_class * overflow;
		overflow = jive_resource_class_count_check_change(&cross_count, rescls, name->resource_class);
		if (overflow)
			continue;
		
		struct jive_variable_interference_hash_iterator j;
		JIVE_HASH_ITERATE(jive_variable_interference_hash, candidate->interference, j) {
			jive_shaped_variable * other = j.entry->shaped_variable;
			
			/* other is colored already or trivially colorable, therefore ignore */
			if (jive_variable_get_resource_name(other->variable) || other->allowed_names.nitems > other->squeeze)
				continue;
			
			if (jive_allowed_resource_names_contains(&other->allowed_names, name)) {
				/* choosing this register would reduce the number
				of choices for another variable */
				pressure  = pressure + 1;
			}
		}
		
		if (pressure < best_pressure) {
			best_name = name;
			best_pressure = pressure;
		}
	}
	
	jive_resource_class_count_fini(&cross_count);
	
	return best_name;
}

static bool
try_assign_name(jive_shaped_graph * shaped_graph, jive_shaped_variable * shaped_variable)
{
	const jive_resource_name * name = find_allowed_name(shaped_graph->graph->context, shaped_variable);
	if (!name)
		return false;
	
	jive_variable_set_resource_name(shaped_variable->variable, name);
	return true;
}

typedef struct regalloc_split_region regalloc_split_region;

struct regalloc_split_region {
	jive_region * region;
	jive_shaped_node * splitting_point;
	struct {
		size_t nitems, space;
		jive_input ** items;
	} users;
	
	struct {
		regalloc_split_region * prev;
		regalloc_split_region * next;
	} chain;
};

static regalloc_split_region *
regalloc_split_region_create(jive_region * region)
{
	jive_context * context = region->graph->context;
	regalloc_split_region * split_region = jive_context_malloc(context, sizeof(*split_region));
	
	split_region->region = region;
	split_region->splitting_point = 0;
	split_region->users.nitems = split_region->users.space = 0;
	split_region->users.items = 0;
	
	return split_region;
}

static void
regalloc_split_region_destroy(regalloc_split_region * self)
{
	jive_context * context = self->region->graph->context;
	jive_context_free(context, self->users.items);
	jive_context_free(context, self);
}

static void
regalloc_split_region_add_user(regalloc_split_region * self, jive_input * user)
{
	if (self->users.nitems == self->users.space) {
		jive_context * context = self->region->graph->context;
		self->users.space = self->users.space * 2 + 1;
		self->users.items = jive_context_realloc(context, self->users.items,
			sizeof(self->users.items[0]) * self->users.space);
	}
	self->users.items[self->users.nitems++] = user;
}

typedef struct split_list {
	regalloc_split_region * first;
	regalloc_split_region * last;
} split_list;

static const jive_resource_class_demotion *
select_split_path(jive_shaped_graph * shaped_graph, const jive_resource_class * rescls, jive_shaped_node * start_point, const split_list splits)
{
	const jive_resource_class * from_rescls = rescls;
	const jive_resource_class_demotion * demotion = from_rescls->demotions;
	while(demotion->target) {
		/* pick one target resource class such that it can be alive over the
		whole range the original value needs to be split over */
		bool allowed = true;
		
		regalloc_split_region * split;
		JIVE_LIST_ITERATE(splits, split, chain) {
			jive_shaped_node * end_point = split->splitting_point;
			
			jive_crossing_arc_iterator i;
			jive_crossing_arc_iterator_init(&i, shaped_graph, start_point, jive_shaped_node_prev_in_region(end_point), end_point->node->region, 0);
			
			while (i.region) {
				if (!i.node)
					continue;
				
				if (jive_resource_class_count_check_add(&i.node->use_count_before, demotion->target)) {
					allowed = false;
					break;
				}
				if (jive_resource_class_count_check_add(&i.node->use_count_after, demotion->target)) {
					allowed = false;
					break;
				}
				jive_crossing_arc_iterator_next(&i);
			}
			
			if (!allowed)
				break;
		}
		if (allowed)
			return demotion;
		demotion ++;
	}
	JIVE_DEBUG_ASSERT(false);
	return 0;
}

static jive_output *
split_top(jive_shaped_graph * shaped_graph, jive_output * origin, const jive_resource_class_demotion * demotion, jive_shaped_node * point)
{
	const jive_resource_class * in_rescls = demotion->path[0];
	size_t n;
	for (n = 1; demotion->path[n]; n++) {
		const jive_resource_class * out_rescls = demotion->path[n];
		const jive_type * in_type = jive_resource_class_get_type(in_rescls);
		const jive_type * out_type = jive_resource_class_get_type(out_rescls);
		
		jive_node * node = jive_aux_split_node_create(origin->node->region,
			in_type, origin, in_rescls,
			out_type, out_rescls);
		
		jive_cut * cut = jive_cut_split(point->cut, jive_shaped_node_next_in_cut(point));
		point = jive_cut_append(cut, node);
		
		jive_input_auto_merge_variable(node->inputs[0]);
		jive_output_auto_merge_variable(node->outputs[0]);
		
		origin = node->outputs[0];
		in_rescls = out_rescls;
	}
	
	return origin;
}

static jive_output *
split_bottom(jive_shaped_graph * shaped_graph, jive_output * origin, const jive_resource_class_demotion * demotion, jive_shaped_node * point)
{
	size_t n = 0;
	while(demotion->path[n+1]) n++;
	
	const jive_resource_class * in_rescls = demotion->path[n];
	while (n != 0) {
		n--;
		const jive_resource_class * out_rescls = demotion->path[n];
		const jive_type * in_type = jive_resource_class_get_type(in_rescls);
		const jive_type * out_type = jive_resource_class_get_type(out_rescls);
		
		jive_node * node = jive_aux_split_node_create(origin->node->region,
			in_type, origin, in_rescls,
			out_type, out_rescls);
		jive_cut * cut = jive_cut_split(point->cut, point);
		point = jive_cut_append(cut, node);
		point = jive_shaped_node_next_in_region(point);
		
		jive_input_auto_merge_variable(node->inputs[0]);
		jive_output_auto_merge_variable(node->outputs[0]);
		
		origin = node->outputs[0];
		in_rescls = out_rescls;
	}
	return origin;
}

static jive_shaped_variable *
lifetime_splitting(jive_shaped_graph * shaped_graph, jive_shaped_variable * shaped_variable)
{
	jive_context * context = shaped_graph->graph->context;
	
	/* split lifetime of SSA variable */
	
	jive_variable * variable = shaped_variable->variable;
	jive_ssavar * ssavar = variable->ssavars.first;
	JIVE_DEBUG_ASSERT(variable->ssavars.first == variable->ssavars.last);
	JIVE_DEBUG_ASSERT(ssavar != 0);
	
	jive_output * origin = ssavar->assigned_output;
	
	split_list splits = {0, 0};
	
	const jive_resource_name * name = 0;
	
	while (!name) {
		jive_input * last_user = 0;
		jive_shaped_node * last_user_loc = 0;
		jive_input * input;
		JIVE_LIST_ITERATE(ssavar->assigned_inputs, input, ssavar_input_list) {
			JIVE_DEBUG_ASSERT(input->ssavar == ssavar);
			jive_shaped_node * loc = jive_shaped_graph_map_node(shaped_graph, input->node);
			jive_shaped_ssavar * shaped_ssavar = jive_shaped_graph_map_ssavar(shaped_graph, ssavar);
			if (!jive_shaped_ssavar_is_crossing(shaped_ssavar, loc)) {
				last_user = input;
				last_user_loc = loc;
				break;
			}
		}
		
		JIVE_DEBUG_ASSERT(last_user && last_user_loc);
		
		regalloc_split_region * split_region;
		JIVE_LIST_ITERATE(splits, split_region, chain) {
			if (split_region->region == last_user->node->region)
				break;
		}
		if (!split_region) {
			split_region = regalloc_split_region_create(last_user->node->region);
			JIVE_LIST_PUSH_FRONT(splits, split_region, chain);
		}
		
		split_region->splitting_point = last_user_loc;
		regalloc_split_region_add_user(split_region, last_user);
			
		/* if split is above sub-regions, users within the sub-regions
		can use the restored value in their parent region and do
		not need a separate split */
		
		regalloc_split_region * other_split, * next_split;
		JIVE_LIST_ITERATE_SAFE(splits, other_split, next_split, chain) {
			if (other_split == split_region)
				continue;
			if (jive_region_is_contained_by(other_split->region, split_region->region)) {
				size_t n;
				for (n = 0; n < other_split->users.nitems; n++)
					regalloc_split_region_add_user(split_region, other_split->users.items[n]);
				JIVE_LIST_REMOVE(splits, other_split, chain);
				regalloc_split_region_destroy(other_split);
			}
		}
		
		jive_input_unassign_ssavar(last_user);
		jive_variable_recompute_rescls(variable);
		name = find_allowed_name(context, shaped_variable);
	}
	
	jive_shaped_node * position = jive_shaped_graph_map_node(shaped_graph, origin->node);
	
	const jive_resource_class_demotion * demotion;
	demotion = select_split_path(shaped_graph, jive_variable_get_resource_class(variable), position, splits);
	
	jive_output * spilled_value;
	spilled_value = split_top(shaped_graph, origin, demotion, position);
	
	while (splits.first) {
		regalloc_split_region * split = splits.first;
		
		jive_output * restored_value = split_bottom(shaped_graph, spilled_value, demotion, split->splitting_point);
		
		size_t n;
		for (n = 0; n < split->users.nitems; n++) {
			jive_input * user = split->users.items[n];
			jive_input_divert_origin(user, restored_value);
		}
		
		jive_output_auto_merge_variable(restored_value);
		
		JIVE_LIST_REMOVE(splits, split, chain);
		regalloc_split_region_destroy(split);
	}
	
	jive_variable_set_resource_name(variable, name);
	
	return 0;
}

static jive_shaped_variable *
ssavar_splitting(jive_shaped_graph * shaped_graph, jive_shaped_variable * shaped_variable)
{
	/* split off singly-defined ssavars from a multiply written variable */
	
	jive_variable * variable = shaped_variable->variable;
	JIVE_DEBUG_ASSERT(!variable->gates.first);
	
	while (variable->ssavars.first != variable->ssavars.last) {
		jive_ssavar * ssavar = variable->ssavars.last;
		
		jive_ssavar_split(ssavar);
		JIVE_DEBUG_ASSERT(ssavar->variable != variable);
		jive_variable_recompute_rescls(ssavar->variable);
		jive_variable_recompute_rescls(variable);
		
		if (try_assign_name(shaped_graph, shaped_variable))
			return 0;
	}
	
	return shaped_variable;
}

static void
merge_gate_ports(jive_gate * gate)
{
	jive_output * output;
	JIVE_LIST_ITERATE(gate->outputs, output, gate_outputs_list)
		jive_output_auto_merge_variable(output);
	
	jive_input * input;
	JIVE_LIST_ITERATE(gate->inputs, input, gate_inputs_list)
		jive_output_auto_merge_variable(input->origin);
}

static jive_shaped_variable *
gate_splitting(jive_shaped_graph * shaped_graph, jive_shaped_variable * shaped_variable)
{
	/* split off gates assigned to variable */
	jive_variable * variable = shaped_variable->variable;
	
	if (!variable->gates.first)
		return shaped_variable;
	
	const jive_resource_class * rescls = jive_variable_get_resource_class(variable);
	const jive_type * type = jive_resource_class_get_type(rescls);
	
	/* insert splitting nodes before and after gates */
	jive_gate * gate;
	JIVE_LIST_ITERATE(variable->gates, gate, variable_gate_list) {
		jive_input * input;
		JIVE_LIST_ITERATE(gate->inputs, input, gate_inputs_list) {
			jive_output * origin = input->origin;
				
			/* don't issue xfer instruction between "tied" gates */
			if (origin->gate == gate) {
				jive_input_unassign_ssavar(input);
				continue;
			}
			
			jive_node * xfer_node = jive_aux_split_node_create(
				input->node->region,
				type, origin, rescls,
				type, rescls);
			
			jive_input * xfer_input = xfer_node->inputs[0];
			jive_output * xfer_output = xfer_node->outputs[0];
			
			jive_input_auto_assign_variable(xfer_input);
			jive_input_unassign_ssavar(input);
			jive_input_divert_origin(input, xfer_output);
			
			/* determine place where to insert node -- try immediately before the gating node,
			but move before predicating node if there is one */
			jive_node * node = input->node;
			size_t n;
			for (n = 0; n < node->ninputs; n++) {
				if (jive_input_isinstance(node->inputs[n], &JIVE_CONTROL_INPUT)) {
					node = node->inputs[n]->origin->node;
					break;
				}
			}
			jive_shaped_node * shaped_node = jive_shaped_graph_map_node(shaped_graph, node);
			
			jive_cut * cut = shaped_node->cut;
			
			cut = jive_cut_split(cut, shaped_node);
			jive_cut_append(cut, xfer_node);
		}
		
		jive_output * output;
		JIVE_LIST_ITERATE(gate->outputs, output, gate_outputs_list) {
			/* don't issue xfer instruction between "tied" gates */
			bool other_user = false;
			jive_input * user;
			JIVE_LIST_ITERATE(output->users, user, output_users_list)
				other_user = other_user || (user->gate != gate);
			if (!other_user) {
				jive_ssavar_unassign_output(output->ssavar, output);
				continue;
			}
			
			jive_node * xfer_node = jive_aux_split_node_create(
				output->node->region,
				type, output, rescls,
				type, rescls);
			
			jive_output * xfer_output = xfer_node->outputs[0];
			
			jive_ssavar_divert_origin(output->ssavar, xfer_output);
			
			/* insert at appropriate place */
			jive_shaped_node * shaped_node = jive_shaped_graph_map_node(shaped_graph, output->node);
			jive_cut * cut = jive_cut_split(shaped_node->cut, jive_shaped_node_next_in_cut(shaped_node));
			
			jive_cut_append(cut, xfer_node);
		}
	}
	
	/* split off SSA variables */
	while (variable->ssavars.first) {
		jive_ssavar * ssavar = variable->ssavars.first;
		jive_ssavar_split(ssavar);
		jive_variable_recompute_rescls(ssavar->variable);
	}
	
	/* now split gates */
	while (variable->gates.first != variable->gates.last) {
		jive_gate * gate = variable->gates.first;
		jive_gate_split(gate);
		merge_gate_ports(gate);
	}
	
	gate = variable->gates.first;
	merge_gate_ports(gate);
	variable = gate->variable;
	
	if (variable)
		shaped_variable = jive_shaped_graph_map_variable(shaped_graph, variable);
	else
		shaped_variable = 0;
	
	if (shaped_variable && try_assign_name(shaped_graph, shaped_variable))
		return 0;
	
	return shaped_variable;
}

static void
jive_regalloc_color_single(jive_shaped_graph * shaped_graph, jive_shaped_variable * shaped_variable)
{
	if (try_assign_name(shaped_graph, shaped_variable))
		shaped_variable = 0;
	if (!shaped_variable)
		return;
	
	shaped_variable = gate_splitting(shaped_graph, shaped_variable);
	if (!shaped_variable)
		return;
	
	shaped_variable = ssavar_splitting(shaped_graph, shaped_variable);
	if (!shaped_variable)
		return;
	
	shaped_variable = lifetime_splitting(shaped_graph, shaped_variable);
	if (!shaped_variable)
		return;
	
	JIVE_DEBUG_ASSERT(false);
}

void
jive_regalloc_color(jive_shaped_graph * shaped_graph)
{
	for(;;) {
		jive_shaped_variable * shaped_variable = find_next_uncolored(&shaped_graph->var_assignment_tracker);
		if (!shaped_variable)
			break;
		
		JIVE_DEBUG_ASSERT(!jive_variable_get_resource_name(shaped_variable->variable));
		
		jive_regalloc_color_single(shaped_graph, shaped_variable);
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