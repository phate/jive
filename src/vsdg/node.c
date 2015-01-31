/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/node-private.h>

#include <string.h>

#include <jive/common.h>

#include <jive/internal/compiler.h>
#include <jive/util/buffer.h>
#include <jive/util/list.h>
#include <jive/vsdg/anchor.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/gate-interference-private.h>
#include <jive/vsdg/graph-private.h>
#include <jive/vsdg/node-normal-form.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/resource-private.h>
#include <jive/vsdg/substitution.h>
#include <jive/vsdg/variable.h>

static void
jive_uninitialized_node_add_output_(jive_node * self, jive::output * output)
{
	JIVE_DEBUG_ASSERT(!self->graph->resources_fully_assigned);
	
	self->noutputs ++;
	self->outputs.resize(self->noutputs);
	self->outputs[output->index] = output;
}

static jive::output *
jive_uninitialized_node_add_output(jive_node * self, const jive::base::type * type)
{
	jive::output * output = type->create_output(self, self->noutputs);
	jive_uninitialized_node_add_output_(self, output);
	return output;
}

static void
jive_uninitialized_node_add_input_(jive_node * self, jive::input * input)
{
	JIVE_DEBUG_ASSERT(!self->graph->resources_fully_assigned);
	
	if (self->ninputs == 0)
		JIVE_LIST_REMOVE(self->region->top_nodes, self, region_top_node_list);

	self->ninputs ++;
	self->inputs.resize(self->ninputs);
	self->inputs[input->index] = input;

}

static jive::input *
jive_uninitialized_node_add_input(jive_node * self, const jive::base::type * type,
	jive::output * initial_operand)
{
	if (self->graph->floating_region_count && !dynamic_cast<const jive::achr::type*>(type)) {
		jive_region * origin_region = initial_operand->node()->region;
		jive_region_check_move_floating(self->region, origin_region);
	}
	
	jive::input * input = jive_type_create_input(type, self, self->ninputs, initial_operand);
	jive_uninitialized_node_add_input_(self, input);

#ifdef JIVE_DEBUG
	jive_region_verify_hull(self->region->graph->root_region);
	jive_region_verify_top_node_list(self->region->graph->root_region);
#endif

	return input;
}

void
jive_node_init_(
	jive_node * self,
	struct jive_region * region,
	size_t noperands,
	const struct jive::base::type * const operand_types[],
	jive::output * const operands[],
	size_t noutputs,
	const struct jive::base::type * const output_types[])
{
	self->graph = region->graph;
	self->depth_from_root = 0;
	self->nsuccessors = 0;
	
	self->ninputs = 0;
	self->noutputs = 0;
	
	JIVE_LIST_PUSH_BACK(region->nodes, self, region_nodes_list);
	self->region = region;
	
	JIVE_LIST_PUSH_BACK(self->region->top_nodes, self, region_top_node_list);
	JIVE_LIST_PUSH_BACK(self->graph->bottom, self, graph_bottom_list);

	size_t n;
	for(n=0; n<noperands; n++) {
		jive_uninitialized_node_add_input(self, operand_types[n], operands[n]);
		if (operands[n]->node()->depth_from_root + 1 > self->depth_from_root)
			 self->depth_from_root = operands[n]->node()->depth_from_root + 1;
	}
	self->noperands = self->ninputs;
	
	for(n=0; n<noutputs; n++)
		jive_uninitialized_node_add_output(self, output_types[n]);
	
	for (n = 0; n < self->ninputs; ++n)
		JIVE_DEBUG_ASSERT(jive_node_valid_edge(self, self->inputs[n]->origin()));
	
	jive_graph_notify_node_create(self->graph, self);
}

void
jive_node_fini_(jive_node * self)
{
	JIVE_DEBUG_ASSERT(self->region);
	
	JIVE_LIST_REMOVE(self->region->nodes, self, region_nodes_list);
	
	while(self->noutputs)
		delete self->outputs[self->noutputs-1];
	
	while (self->ninputs)
		delete self->inputs[self->ninputs-1];
	
	JIVE_LIST_REMOVE(self->graph->bottom, self, graph_bottom_list);
	JIVE_LIST_REMOVE(self->region->top_nodes, self, region_top_node_list);

	if (self == self->region->top) self->region->top = NULL;
	if (self == self->region->bottom) self->region->bottom = NULL;
	
	self->region = 0;
	
	for (size_t n = 0; n < self->tracker_slots.size(); n++)
		delete self->tracker_slots[n];
}

jive::node_normal_form *
jive_node_get_default_normal_form_(
	const std::type_info & operator_class,
	jive::node_normal_form * parent,
	jive_graph * graph)
{
	jive::node_normal_form * normal_form = new jive::node_normal_form(
		operator_class, parent, graph);
	return normal_form;
}

static void  __attribute__((constructor))
register_node_normal_form(void)
{
	jive::node_normal_form::register_factory(
		typeid(jive::operation), jive_node_get_default_normal_form_);
}

static void
jive_node_add_input_(jive_node * self, jive::input * input)
{
	jive_uninitialized_node_add_input_(self, input);

	JIVE_DEBUG_ASSERT(jive_node_valid_edge(self, input->origin()));
	jive_node_invalidate_depth_from_root(self);
	jive_graph_notify_input_create(self->graph, input);
}

bool
jive_node_valid_edge(const jive_node * self, const jive::output * origin)
{
	jive_region * origin_region = origin->node()->region;
	jive_region * target_region = self->region;
	if (dynamic_cast<const jive::achr::type*>(&origin->type()))
		origin_region = origin_region->parent;
	while (target_region) {
		if (target_region == origin_region)
			return true;
		target_region = target_region->parent;
	}
	return false;
}

jive::input *
jive_node_add_input(jive_node * self, const jive::base::type * type, jive::output * initial_operand)
{
	if (self->graph->floating_region_count && dynamic_cast<const jive::achr::type*>(type)) {
		jive_region * origin_region = initial_operand->node()->region;
		jive_region_check_move_floating(self->region, origin_region);
	}

	jive::input * input = jive_type_create_input(type, self, self->ninputs, initial_operand);
	jive_node_add_input_(self, input);

#ifdef JIVE_DEBUG
	jive_region_verify_hull(self->region->graph->root_region);
	jive_region_verify_top_node_list(self->region->graph->root_region);
#endif

	return input;
}

static void
jive_node_add_output_(jive_node * self, jive::output * output)
{
	jive_uninitialized_node_add_output_(self, output);
	
	if (self->region) jive_graph_notify_output_create(self->graph, output);
}

jive::output *
jive_node_add_output(jive_node * self, const jive::base::type * type)
{
	jive::output * output = type->create_output(self, self->noutputs);
	jive_node_add_output_(self, output);
	return output;
}

jive::output *
jive_node_add_constrained_output(jive_node * self, const jive_resource_class * rescls)
{
	jive::output * output = jive_node_add_output(self, jive_resource_class_get_type(rescls));
	output->required_rescls = rescls;
	return output;
}

jive::input *
jive_node_add_constrained_input(jive_node * self, const jive_resource_class * rescls,
	jive::output * initial_operand)
{
	jive::input * input = jive_node_add_input(self, jive_resource_class_get_type(rescls),
		initial_operand);
	input->required_rescls = rescls;
	return input;
}

jive::input *
jive_node_gate_input(jive_node * self, jive::gate * gate, jive::output * initial_operand)
{
	if (self->graph->floating_region_count) {
		jive_region * origin_region = initial_operand->node()->region;
		jive_region_check_move_floating(self->region, origin_region);
	}

	jive::input * input = gate->create_input(self, self->ninputs, initial_operand);
	size_t n;
	for(n=0; n<input->index; n++) {
		jive::input * other = self->inputs[n];
		if (!other->gate) continue;
		jive_gate_interference_add(self->graph, gate, other->gate);
	}
	jive_node_add_input_(self, input);
	return input;
}

jive::output *
jive_node_gate_output(jive_node * self, jive::gate * gate)
{
	jive::output * output = gate->create_output(self, self->noutputs);
	size_t n;
	for(n=0; n<output->index; n++) {
		jive::output * other = self->outputs[n];
		if (!other->gate) continue;
		jive_gate_interference_add(self->graph, gate, other->gate);
	}
	jive_node_add_output_(self, output);
	return output;
}

jive::input *
jive_node_input(const jive_node * self, size_t index)
{
	jive::input * input = NULL;
	if (index < self->ninputs) {
		input = self->inputs[index];
	} else {
		throw std::logic_error("Input index out of bound.");
	}

	return input;
}

jive::output *
jive_node_output(const jive_node * self, size_t index)
{
	jive::output * output = NULL;
	if (index < self->noutputs) {
		return self->outputs[index];
	} else {
		throw std::logic_error("Output index out of bound.");
	}

	return output;
}

void
jive_node_add_successor(jive_node * self)
{
	if (unlikely(self->nsuccessors == 0))
		JIVE_LIST_REMOVE(self->graph->bottom, self, graph_bottom_list);
	
	self->nsuccessors ++;
}

void
jive_node_remove_successor(jive_node * self)
{
	self->nsuccessors --;
	if (unlikely(self->nsuccessors == 0))
		JIVE_LIST_PUSH_BACK(self->graph->bottom, self, graph_bottom_list);
}

void
jive_node_invalidate_depth_from_root(jive_node * self)
{
	size_t new_depth_from_root = 0, n;
	for(n=0; n<self->ninputs; n++)
		if (self->producer(n)->depth_from_root + 1 > new_depth_from_root)
			 new_depth_from_root = self->producer(n)->depth_from_root + 1;
	
	size_t old_depth_from_root = self->depth_from_root;
	if (old_depth_from_root == new_depth_from_root)
		return;
	self->depth_from_root = new_depth_from_root;
	
	jive_node_depth_notifier_slot_call(&self->graph->on_node_depth_change, self, old_depth_from_root);
	
	for(n=0; n<self->noutputs; n++) {
		jive::input * user = self->outputs[n]->users.first;
		while(user) {
			jive_node_invalidate_depth_from_root(user->node);
			user = user->output_users_list.next;
		}
	}
}

void
jive_node_auto_merge_variables(jive_node * self)
{
	size_t n;
	for(n = 0; n < self->ninputs; n++)
		jive_input_auto_merge_variable(self->inputs[n]);
	for(n = 0; n < self->noutputs; n++)
		jive_output_auto_merge_variable(self->outputs[n]);
}

void
jive_node_get_use_count_input(const jive_node * self, jive_resource_class_count * use_count)
{
	jive_resource_class_count_clear(use_count);
	
	size_t n;
	for(n = 0; n<self->ninputs; n++) {
		jive::input * input = self->inputs[n];
		
		/* filter out multiple inputs using the same value
		FIXME: this assumes that all inputs have the same resource
		class requirement! */
		if (!input->gate) {
			bool duplicate = false;
			size_t k;
			for(k = 0; k<n; k++) {
				if (self->inputs[k]->origin() == input->origin())
					duplicate = true;
			}
			if (duplicate) continue;
		}
		
		const jive_resource_class * rescls;
		if (input->ssavar) rescls = input->ssavar->variable->rescls;
		else if (input->gate) rescls = input->gate->required_rescls;
		else rescls = input->required_rescls;
		
		jive_resource_class_count_add(use_count, rescls);
	}
}

void
jive_node_get_use_count_output(const jive_node * self, jive_resource_class_count * use_count)
{
	jive_resource_class_count_clear(use_count);
	
	size_t n;
	for(n = 0; n<self->noutputs; n++) {
		jive::output * output = self->outputs[n];
		
		const jive_resource_class * rescls;
		if (output->ssavar) rescls = output->ssavar->variable->rescls;
		else if (output->gate) rescls = output->gate->required_rescls;
		else rescls = output->required_rescls;
		
		jive_resource_class_count_add(use_count, rescls);
	}
}

/**
	\brief Test whether node has inputs from region
*/
bool
jive_node_depends_on_region(const jive_node * self, const jive_region * region)
{
	size_t n;
	for(n = 0; n < self->ninputs; n++) {
		jive::input * input = self->inputs[n];
		if (dynamic_cast<const jive::achr::type*>(&input->type())) {
			if (jive_region_depends_on_region(input->producer()->region, region)) {
				return true;
			}
		} else {
			if (input->producer()->region == region) {
				return true;
			}
		}
	}
	
	return false;
}

bool
jive_node_can_move_outward(const jive_node * self)
{
	return self->region->parent
		&& self->region->top != self
		&& self->region->bottom != self
		&& !jive_node_depends_on_region(self, self->region);
}

void
jive_node_move_outward(jive_node * self)
{
	jive_node_move(self, self->region->parent);
}

bool
jive_node_can_move_inward(const jive_node * self)
{
	return jive_node_next_inner_region(self) != NULL;
}

struct jive_region *
jive_node_next_inner_region(const jive_node * self)
{
	jive_region * current = self->region;
	if (current->top == self || current->bottom == self)
		return NULL;
	jive_region * target = NULL;
	size_t n;
	for (n = 0; n < self->noutputs; n++) {
		jive::output * output = self->outputs[n];
		jive::input * user;
		JIVE_LIST_ITERATE(output->users, user, output_users_list) {
			jive_region * region = user->node->region;
			/* cannot pull anywhere if dependence in same region */
			if (region == current)
				return NULL;
			while (region->parent != current)
				region = region->parent;
			/* can only pull if all dependencies in same region (or deeper) */
			if (target && target != region)
				return NULL;
			/* don't pull into looped or lambda def regions */
			if (region->top)
				return NULL;
			target = region;
		}
	}
	return target;
}

void
jive_node_move_inward(jive_node * self)
{
	jive_region * target = jive_node_next_inner_region(self);
	if (!target)
		return;
	jive_node_move(self, target);
}

void
jive_node_move(jive_node * self, jive_region * new_region)
{
	if (self->region == new_region)
		return;

	/* verify that the node is moved along the region path to the root */
	jive_region * child = new_region;
	jive_region * parent = self->region;
	if (self->region->depth > new_region->depth) {
		child = self->region;
		parent = new_region;
	}
	if (!jive_region_is_contained_by(child, parent))
		throw std::logic_error("Node can only be moved along the region path to the root.");

	size_t n;
	/* remove all node inputs from hull of old region and update notion of
	top nodes of old region */
	for (n = 0; n < self->ninputs; n++)
		jive_region_hull_remove_input(self->region, self->inputs[n]);
	if (self->ninputs == 0) {
		JIVE_LIST_REMOVE(self->region->top_nodes, self, region_top_node_list);
	}
		
	/* remove all node output users in new region from the hulls */
	for (n = 0; n < self->noutputs; n++) {
		jive::input * user;
		JIVE_LIST_ITERATE(self->outputs[n]->users, user, output_users_list)
			jive_region_hull_remove_input(user->node->region, user);
	}


	/* move the node to the new region */
	JIVE_LIST_REMOVE(self->region->nodes, self, region_nodes_list);
	self->region = new_region;
	JIVE_LIST_PUSH_BACK(self->region->nodes, self, region_nodes_list);

	/* re-add all node inputs to hull of new region and update notion
	of top nodes of new region */
	for (n = 0; n < self->ninputs; n++) {
		/* if it is an anchor node, we also need to pull/push in/out the corresponding regions */
		if (dynamic_cast<const jive::achr::type*>(&self->inputs[n]->type())) {
			jive_region * subregion = self->producer(n)->region;
			jive_region_reparent(subregion, new_region);
		} else if (self->producer(n)->region != new_region) {
			/* or add the node's input to the hull */
			jive_region_hull_add_input(new_region, self->inputs[n]);
		}
	}
	if (self->ninputs == 0) {
		JIVE_LIST_PUSH_BACK(self->region->top_nodes, self, region_top_node_list);
	}


	/* add all output users to the hulls */
	for (n = 0; n < self->noutputs; n++) {
		jive::input * user;
		JIVE_LIST_ITERATE(self->outputs[n]->users, user, output_users_list) {
			if (self->region != user->node->region)
				jive_region_hull_add_input(user->node->region, user);
		}
	}
}

struct jive_node *
jive_node_copy(const jive_node * self, struct jive_region * region, jive::output * operands[])
{
	jive_graph_mark_denormalized(region->graph);
	return self->operation().create_node(region, self->noperands, operands);
}

jive_node *
jive_node_copy_substitute(const jive_node * self, jive_region * target,
	jive_substitution_map * substitution)
{
	jive::output * operands[self->noperands];
	
	size_t n;
	for(n = 0; n < self->noperands; n++) {
		operands[n] = self->inputs[n]->origin();
		jive::output * tmp = jive_substitution_map_lookup_output(substitution, self->inputs[n]->origin());
		if (tmp) operands[n] = tmp;
	}
	
	jive_node * new_node = jive_node_copy(self, target, operands);
	for(n = self->noperands; n < self->ninputs; n++) {
		jive::output * origin = self->inputs[n]->origin();
		jive::output * tmp = jive_substitution_map_lookup_output(substitution, self->inputs[n]->origin());
		if (tmp) origin = tmp;
		
		if (self->inputs[n]->gate) {
			jive::gate * gate = self->inputs[n]->gate;
			jive::gate * target_gate = jive_substitution_map_lookup_gate(substitution, gate);
			if (!target_gate) {
				target_gate = gate->type().create_gate(target->graph, gate->name.c_str());
				target_gate->required_rescls = gate->required_rescls;
				jive_substitution_map_add_gate(substitution, gate, target_gate);
			}
			jive_node_gate_input(new_node, target_gate, origin);
		} else {
			jive::input * input = jive_node_add_input(new_node, &self->inputs[n]->type(), origin);
			input->required_rescls = self->inputs[n]->required_rescls;
		}
	}
	
	for(n = new_node->noutputs; n < self->noutputs; n++) {
		if (self->outputs[n]->gate) {
			jive::gate * gate = self->outputs[n]->gate;
			jive::gate * target_gate = jive_substitution_map_lookup_gate(substitution, gate);
			if (!target_gate) {
				target_gate = gate->type().create_gate(target->graph, gate->name.c_str());
				target_gate->required_rescls = gate->required_rescls;
				jive_substitution_map_add_gate(substitution, gate, target_gate);
			}
			jive_node_gate_output(new_node, target_gate);
		} else {
			jive::output * output = jive_node_add_output(new_node, &self->outputs[n]->type());
			output->required_rescls = self->outputs[n]->required_rescls;
		}
	}
	
	for(n = 0; n < new_node->noutputs; n++)
		jive_substitution_map_add_output(substitution, self->outputs[n], new_node->outputs[n]);
	
	return new_node;
}

void
jive_node_destroy(jive_node * self)
{
	jive_graph_notify_node_destroy(self->graph, self);
	jive_node_fini_(self);
	delete self;
}

static bool
jive_node_cse_test(
	jive_node * node,
	const jive::operation & op,
	const std::vector<jive::output *> & arguments)
{
	return (node->operation() == op && arguments == jive_node_arguments(node));
}

jive_node *
jive_node_cse(
	jive_region * region,
	const jive::operation & op,
	const std::vector<jive::output *> & arguments)
{
	if (!arguments.empty()) {
		jive::input * input;
		JIVE_LIST_ITERATE(arguments[0]->users, input, output_users_list) {
			jive_node * node = input->node;
			if (jive_node_cse_test(node, op, arguments)) {
				return node;
			}
		}
	} else {
		while (region) {
			jive_node * node;
			JIVE_LIST_ITERATE(region->top_nodes, node, region_top_node_list)
				if (jive_node_cse_test(node, op, arguments)) {
					return node;
				}
			region = region->parent;
		}
	}

	return nullptr;
}

std::vector<jive::output *>
jive_node_create_normalized(
	jive_graph * graph,
	const jive::operation & op,
	const std::vector<jive::output *> & arguments)
{
	jive::node_normal_form * nf = jive_graph_get_nodeclass_form(graph, typeid(op));
	return nf->normalized_create(op, arguments);
}

jive_tracker_nodestate *
jive_node_get_tracker_state_slow(jive_node * self, jive_tracker_slot slot)
{
	size_t new_size = slot.index + 1;
	
	size_t ntracker_slots = self->tracker_slots.size();
	self->tracker_slots.resize(new_size);
	
	jive_tracker_nodestate * nodestate;
	for(size_t n = ntracker_slots; n < new_size; n++) {
		nodestate = new jive_tracker_nodestate;
		nodestate->node = self;
		nodestate->cookie = 0;
		nodestate->state = jive_tracker_nodestate_none;
		nodestate->tag = 0;
		self->tracker_slots[n] = nodestate;
	}
	
	nodestate = self->tracker_slots[slot.index];
	nodestate->cookie = slot.cookie;
	nodestate->state = jive_tracker_nodestate_none;
	nodestate->tag = 0;
	
	return nodestate;
}

jive_node *
jive_node_cse_create(
	const jive::node_normal_form * nf,
	jive_region * region,
	const jive::operation & op,
	const std::vector<jive::output *> & arguments)
{
	jive_node * node;
	if (nf->get_mutable() && nf->get_cse()) {
		node = jive_node_cse(region, op, arguments);
		if (node) {
			return node;
		}
	}

	return op.create_node(region, arguments.size(), &arguments[0]);
}

bool
jive_node_normalize(jive_node * self)
{
	jive_graph * graph = self->region->graph;
	const jive::node_normal_form * nf = jive_graph_get_nodeclass_form(
		graph, typeid(self->operation()));
	return nf->normalize_node(self);
}

jive_node *
jive_opnode_create(
	const jive::operation & op,
	jive_region * region,
	jive::output * const * args_begin,
	jive::output * const * args_end)
{
	const jive::base::type * argument_types[op.narguments()];
	jive::output * argument_values[op.narguments()];
	for (size_t n = 0; n < op.narguments(); ++n) {
		argument_types[n] = &op.argument_type(n);
		JIVE_DEBUG_ASSERT(args_begin != args_end);
		argument_values[n] = *args_begin;
		++args_begin;
	}
	JIVE_DEBUG_ASSERT(args_begin == args_end);
	
	const jive::base::type * result_types[op.nresults()];
	for (size_t n = 0; n < op.nresults(); ++n) {
		result_types[n] = &op.result_type(n);
	}
	jive_node * node = jive::create_operation_node(op);
	jive_node_init_(node, region,
		op.narguments(), argument_types, argument_values,
		op.nresults(), result_types);

	/* FIXME: this is regalloc-specific, should go away */
	for (size_t n = 0; n < op.narguments(); ++n) {
		node->inputs[n]->required_rescls = op.argument_cls(n);
	}
	for (size_t n = 0; n < op.nresults(); ++n) {
		node->outputs[n]->required_rescls = op.result_cls(n);
	}

	/* FIXME: region head/tail nodes are a bit quirky, but they
	 * will go away eventually anyways */
	if (dynamic_cast<const jive::region_head_op *>(&op)) {
		JIVE_DEBUG_ASSERT(!region->top);
		region->top = node;
	} else if (dynamic_cast<const jive::region_tail_op *>(&op)) {
		JIVE_DEBUG_ASSERT(!region->bottom);
		region->bottom = node;
	}

	return node;
}
