/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/node-private.h>

#include <string.h>

#include <jive/common.h>

#include <jive/internal/compiler.h>
#include <jive/util/buffer.h>
#include <jive/util/list.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/gate-interference-private.h>
#include <jive/vsdg/graph-private.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/resource-private.h>
#include <jive/vsdg/substitution.h>
#include <jive/vsdg/variable.h>

jive_node::~jive_node() noexcept {}

const jive_node_attrs &
jive_node::operation() const noexcept
{
	return *class_->get_attrs(this);
}

const jive_node_class JIVE_NODE = {
	parent : 0,
	name : "NODE",
	fini : jive_node_fini_,
	get_default_normal_form : jive_node_get_default_normal_form_,
	get_label : jive_node_get_label_,
	get_attrs : jive_node_get_attrs_,
	match_attrs : jive_node_match_attrs_,
	check_operands : jive_node_check_operands_,
	create : jive_node_create_,
};

static void
jive_uninitialized_node_add_output_(jive_node * self, jive_output * output)
{
	JIVE_DEBUG_ASSERT(!self->graph->resources_fully_assigned);
	
	self->noutputs ++;
	self->outputs = jive_context_realloc(self->graph->context, self->outputs,
		sizeof(jive_output *) * self->noutputs);
	self->outputs[output->index] = output;
}

static jive_output *
jive_uninitialized_node_add_output(jive_node * self, const jive_type * type)
{
	jive_output * output = jive_type_create_output(type, self, self->noutputs);
	jive_uninitialized_node_add_output_(self, output);
	return output;
}

static void
jive_uninitialized_node_add_input_(jive_node * self, jive_input * input)
{
	JIVE_DEBUG_ASSERT(!self->graph->resources_fully_assigned);
	
	if (self->ninputs == 0)
		JIVE_LIST_REMOVE(self->region->top_nodes, self, region_top_node_list);

	self->ninputs ++;
	self->inputs = jive_context_realloc(self->graph->context, self->inputs,
		sizeof(jive_input *) * self->ninputs);
	self->inputs[input->index] = input;

}

static jive_input *
jive_uninitialized_node_add_input(jive_node * self, const jive_type * type,
	jive_output * initial_operand)
{
	if (self->graph->floating_region_count && type->class_ != &JIVE_ANCHOR_TYPE) {
		jive_region * origin_region = initial_operand->node->region;
		jive_region_check_move_floating(self->region, origin_region);
	}
	
	jive_input * input = jive_type_create_input(type, self, self->ninputs, initial_operand);
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
	const struct jive_type * const operand_types[],
	struct jive_output * const operands[],
	size_t noutputs,
	const struct jive_type * const output_types[])
{
	self->graph = region->graph;
	self->depth_from_root = 0;
	self->nsuccessors = 0;
	
	self->ninputs = 0;
	self->inputs = 0;
	
	self->noutputs = 0;
	self->outputs = 0;
	
	JIVE_LIST_PUSH_BACK(region->nodes, self, region_nodes_list);
	self->region = region;
	
	JIVE_LIST_PUSH_BACK(self->region->top_nodes, self, region_top_node_list);
	JIVE_LIST_PUSH_BACK(self->graph->bottom, self, graph_bottom_list);
	
	size_t n;
	for(n=0; n<noperands; n++) {
		jive_uninitialized_node_add_input(self, operand_types[n], operands[n]);
		if (operands[n]->node->depth_from_root + 1 > self->depth_from_root)
			 self->depth_from_root = operands[n]->node->depth_from_root + 1;
	}
	self->noperands = self->ninputs;
	
	for(n=0; n<noutputs; n++)
		jive_uninitialized_node_add_output(self, output_types[n]);
	
	self->ntraverser_slots = 0;
	self->traverser_slots = 0;
	
	self->ntracker_slots = 0;
	self->tracker_slots = 0;
	
	for (n = 0; n < self->ninputs; ++n)
		JIVE_DEBUG_ASSERT(jive_node_valid_edge(self, self->inputs[n]->origin()));
	
	jive_graph_notify_node_create(self->graph, self);
}

void
jive_node_fini_(jive_node * self)
{
	jive_context * context = self->graph->context;
	JIVE_DEBUG_ASSERT(self->region);
	
	JIVE_LIST_REMOVE(self->region->nodes, self, region_nodes_list);
	
	while(self->noutputs) jive_output_destroy(self->outputs[self->noutputs - 1]);
	
	while (self->ninputs)
		delete self->inputs[self->ninputs-1];
	
	JIVE_LIST_REMOVE(self->graph->bottom, self, graph_bottom_list);
	JIVE_LIST_REMOVE(self->region->top_nodes, self, region_top_node_list);

	if (self == self->region->top) self->region->top = NULL;
	if (self == self->region->bottom) self->region->bottom = NULL;
	
	self->region = 0;
	jive_context_free(context, self->inputs);
	jive_context_free(context, self->outputs);
	
	if (self->traverser_slots) {
		size_t n;
		for(n=0; n<self->ntraverser_slots; n++)
			jive_context_free(context, self->traverser_slots[n]);
		jive_context_free(context, self->traverser_slots);
	}
	
	if (self->tracker_slots) {
		size_t n;
		for(n=0; n<self->ntracker_slots; n++)
			jive_context_free(context, self->tracker_slots[n]);
		jive_context_free(context, self->tracker_slots);
	}
}

void
jive_node_get_label_(const jive_node * self, struct jive_buffer * buffer)
{
	jive_buffer_putstr(buffer, self->class_->name);
}

bool
jive_node_match_attrs_(const jive_node * self, const jive_node_attrs * other)
{
	return true;
}

const jive_node_attrs *
jive_node_get_attrs_(const jive_node * self)
{
	return 0;
}

void
jive_node_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[], jive_context * context)
{
	if (cls->parent == &JIVE_NODE)
		return;

	if (noperands == 0)
		return;

	jive_context_fatal_error(context, "Checking of node operands failed.");
}

jive_node *
jive_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[])
{
	jive_node * other = new jive_node;
	const jive_type * operand_types[noperands];
	size_t n;
	for(n=0; n<noperands; n++)
		operand_types[n] = jive_output_get_type(operands[n]);
	
	other->class_ = &JIVE_NODE;
	jive_node_init_(other, region,
		noperands, operand_types, operands,
		0, 0);
	
	return other;
}

jive_node_normal_form *
jive_node_get_default_normal_form_(const jive_node_class * cls, jive_node_normal_form * parent,
	jive_graph * graph)
{
	jive_node_normal_form * normal_form;
	normal_form = jive_context_malloc(graph->context, sizeof(*normal_form));
	normal_form->class_ = &JIVE_NODE_NORMAL_FORM;
	jive_node_normal_form_init_(normal_form, cls, parent, graph);
	return normal_form;
}

jive_node *
jive_node_create(
	struct jive_region * region,
	size_t noperands,
	const struct jive_type * const * operand_types,
	struct jive_output * const * operands,
	size_t noutputs,
	const struct jive_type * const * output_types)
{
	jive_node * node = new jive_node;
	node->class_ = &JIVE_NODE;
	jive_node_init_(node, region, noperands, operand_types, operands, noutputs, output_types);
	
	return node;
}

static void
jive_node_add_input_(jive_node * self, jive_input * input)
{
	jive_uninitialized_node_add_input_(self, input);

	JIVE_DEBUG_ASSERT(jive_node_valid_edge(self, input->origin()));
	jive_node_invalidate_depth_from_root(self);
	jive_graph_notify_input_create(self->graph, input);
}

bool
jive_node_valid_edge(const jive_node * self, const jive_output * origin)
{
	jive_region * origin_region = origin->node->region;
	jive_region * target_region = self->region;
	if (dynamic_cast<const jive_anchor_output*>(origin))
		origin_region = origin_region->parent;
	while (target_region) {
		if (target_region == origin_region)
			return true;
		target_region = target_region->parent;
	}
	return false;
}

jive_input *
jive_node_add_input(jive_node * self, const jive_type * type, jive_output * initial_operand)
{
	if (self->graph->floating_region_count && type->class_ != &JIVE_ANCHOR_TYPE) {
		jive_region * origin_region = initial_operand->node->region;
		jive_region_check_move_floating(self->region, origin_region);
	}

	jive_input * input = jive_type_create_input(type, self, self->ninputs, initial_operand);
	jive_node_add_input_(self, input);

#ifdef JIVE_DEBUG
	jive_region_verify_hull(self->region->graph->root_region);
	jive_region_verify_top_node_list(self->region->graph->root_region);
#endif

	return input;
}

static void
jive_node_add_output_(jive_node * self, jive_output * output)
{
	jive_uninitialized_node_add_output_(self, output);
	
	if (self->region) jive_graph_notify_output_create(self->graph, output);
}

jive_output *
jive_node_add_output(jive_node * self, const jive_type * type)
{
	jive_output * output = jive_type_create_output(type, self, self->noutputs);
	jive_node_add_output_(self, output);
	return output;
}

jive_output *
jive_node_add_constrained_output(jive_node * self, const jive_resource_class * rescls)
{
	jive_output * output = jive_node_add_output(self, jive_resource_class_get_type(rescls));
	output->required_rescls = rescls;
	return output;
}

jive_input *
jive_node_add_constrained_input(jive_node * self, const jive_resource_class * rescls,
	jive_output * initial_operand)
{
	jive_input * input = jive_node_add_input(self, jive_resource_class_get_type(rescls),
		initial_operand);
	input->required_rescls = rescls;
	return input;
}

jive_input *
jive_node_gate_input(jive_node * self, jive_gate * gate, jive_output * initial_operand)
{
	if (self->graph->floating_region_count) {
		jive_region * origin_region = initial_operand->node->region;
		jive_region_check_move_floating(self->region, origin_region);
	}

	jive_input * input = jive_gate_create_input(gate, self, self->ninputs, initial_operand);
	input->gate = gate;
	JIVE_LIST_PUSH_BACK(gate->inputs, input, gate_inputs_list);
	size_t n;
	for(n=0; n<input->index; n++) {
		jive_input * other = self->inputs[n];
		if (!other->gate) continue;
		jive_gate_interference_add(self->graph, gate, other->gate);
	}
	jive_node_add_input_(self, input);
	return input;
}

jive_output *
jive_node_gate_output(jive_node * self, jive_gate * gate)
{
	jive_output * output = jive_gate_create_output(gate, self, self->noutputs);
	output->gate = gate;
	JIVE_LIST_PUSH_BACK(gate->outputs, output, gate_outputs_list);
	size_t n;
	for(n=0; n<output->index; n++) {
		jive_output * other = self->outputs[n];
		if (!other->gate) continue;
		jive_gate_interference_add(self->graph, gate, other->gate);
	}
	jive_node_add_output_(self, output);
	return output;
}

struct jive_input *
jive_node_input(const struct jive_node * self, size_t index)
{
	jive_input * input = NULL;
	if (index < self->ninputs)
		input = self->inputs[index];
	else
		jive_context_fatal_error(self->graph->context, "Input index out of bound.");

	return input;
}

struct jive_output *
jive_node_output(const struct jive_node * self, size_t index)
{
	jive_output * output = NULL;
	if (index < self->noutputs)
		return self->outputs[index];
	else
		jive_context_fatal_error(self->graph->context, "Output index out of bound.");

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
		if (self->inputs[n]->origin()->node->depth_from_root + 1 > new_depth_from_root)
			 new_depth_from_root = self->inputs[n]->origin()->node->depth_from_root + 1;
	
	size_t old_depth_from_root = self->depth_from_root;
	if (old_depth_from_root == new_depth_from_root)
		return;
	self->depth_from_root = new_depth_from_root;
	
	jive_node_depth_notifier_slot_call(&self->graph->on_node_depth_change, self, old_depth_from_root);
	
	for(n=0; n<self->noutputs; n++) {
		jive_input * user = self->outputs[n]->users.first;
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
jive_node_get_use_count_input(const jive_node * self, jive_resource_class_count * use_count,
	jive_context * context)
{
	jive_resource_class_count_clear(use_count);
	
	size_t n;
	for(n = 0; n<self->ninputs; n++) {
		jive_input * input = self->inputs[n];
		
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
jive_node_get_use_count_output(const jive_node * self, jive_resource_class_count * use_count,
	jive_context * context)
{
	jive_resource_class_count_clear(use_count);
	
	size_t n;
	for(n = 0; n<self->noutputs; n++) {
		jive_output * output = self->outputs[n];
		
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
		jive_input * input = self->inputs[n];
		if (dynamic_cast<jive_anchor_input*>(input)) {
			if (jive_region_depends_on_region(input->origin()->node->region, region)) {
				return true;
			}
		} else {
			if (input->origin()->node->region == region) {
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
		jive_output * output = self->outputs[n];
		jive_input * user;
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
		jive_context_fatal_error(self->region->graph->context,
			"Node can only be moved along the region path to the root.");

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
		jive_input * user;
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
		if (dynamic_cast<jive_anchor_input*>(self->inputs[n])) {
			jive_region * subregion = self->inputs[n]->origin()->node->region;
			jive_region_reparent(subregion, new_region);
		} else if (self->inputs[n]->origin()->node->region != new_region) {
			/* or add the node's input to the hull */
			jive_region_hull_add_input(new_region, self->inputs[n]);
		}
	}
	if (self->ninputs == 0) {
		JIVE_LIST_PUSH_BACK(self->region->top_nodes, self, region_top_node_list);
	}


	/* add all output users to the hulls */
	for (n = 0; n < self->noutputs; n++) {
		jive_input * user;
		JIVE_LIST_ITERATE(self->outputs[n]->users, user, output_users_list) {
			if (self->region != user->node->region)
				jive_region_hull_add_input(user->node->region, user);
		}
	}
}

struct jive_node *
jive_node_copy(const jive_node * self, struct jive_region * region, struct jive_output * operands[])
{
	jive_graph_mark_denormalized(region->graph);
	return self->class_->create(region, jive_node_get_attrs(self), self->noperands, operands);
}

jive_node *
jive_node_copy_substitute(const jive_node * self, jive_region * target,
	jive_substitution_map * substitution)
{
	jive_output * operands[self->noperands];
	
	size_t n;
	for(n = 0; n < self->noperands; n++) {
		operands[n] = self->inputs[n]->origin();
		jive_output * tmp = jive_substitution_map_lookup_output(substitution, self->inputs[n]->origin());
		if (tmp) operands[n] = tmp;
	}
	
	jive_node * new_node = jive_node_copy(self, target, operands);
	for(n = self->noperands; n < self->ninputs; n++) {
		jive_output * origin = self->inputs[n]->origin();
		jive_output * tmp = jive_substitution_map_lookup_output(substitution, self->inputs[n]->origin());
		if (tmp) origin = tmp;
		
		if (self->inputs[n]->gate) {
			jive_gate * gate = self->inputs[n]->gate;
			jive_gate * target_gate = jive_substitution_map_lookup_gate(substitution, gate);
			if (!target_gate) {
				target_gate = jive_type_create_gate(jive_gate_get_type(gate), target->graph, gate->name);
				target_gate->required_rescls = gate->required_rescls;
				jive_substitution_map_add_gate(substitution, gate, target_gate);
			}
			jive_node_gate_input(new_node, target_gate, origin);
		} else {
			jive_input * input = jive_node_add_input(new_node, jive_input_get_type(self->inputs[n]), origin);
			input->required_rescls = self->inputs[n]->required_rescls;
		}
	}
	
	for(n = new_node->noutputs; n < self->noutputs; n++) {
		if (self->outputs[n]->gate) {
			jive_gate * gate = self->outputs[n]->gate;
			jive_gate * target_gate = jive_substitution_map_lookup_gate(substitution, gate);
			if (!target_gate) {
				target_gate = jive_type_create_gate(jive_gate_get_type(gate), target->graph, gate->name);
				target_gate->required_rescls = gate->required_rescls;
				jive_substitution_map_add_gate(substitution, gate, target_gate);
			}
			jive_node_gate_output(new_node, target_gate);
		} else {
			jive_output * output = jive_node_add_output(new_node, jive_output_get_type(self->outputs[n]));
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
	self->class_->fini(self);
	delete self;
}

static bool
jive_node_cse_test(
	jive_node * node,
	const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[])
{
	if (node->class_ != cls) return false;
	if (node->ninputs != noperands) return false;
	if (!jive_node_match_attrs(node, attrs)) return false;
	size_t n;
	for(n = 0; n < noperands; n++)
		if (node->inputs[n]->origin() != operands[n]) return false;
	
	return true;
}

jive_node *
jive_node_cse(
	jive_region * region,
	const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[])
{
	if (noperands) {
		jive_input * input;
		JIVE_LIST_ITERATE(operands[0]->users, input, output_users_list) {
			jive_node * node = input->node;
			if (jive_node_cse_test(node, cls, attrs, noperands, operands))
				return node;
		}
	} else {
		while (region) {
			jive_node * node;
			JIVE_LIST_ITERATE(region->top_nodes, node, region_top_node_list)
				if (jive_node_cse_test(node, cls, attrs, noperands, operands))
					return node;
			region = region->parent;
		}
	}

	return NULL;
}

void
jive_node_create_normalized(const jive_node_class * class_, struct jive_graph * graph,
	const jive_node_attrs * attrs, size_t noperands, jive_output * const operands[],
	jive_output * results[])
{
	jive_node_check_operands(class_, attrs, noperands, operands, graph->context);
	jive_node_normal_form * nf = jive_graph_get_nodeclass_form(graph, class_);
	jive_node_normal_form_normalized_create(nf, graph, attrs, noperands, operands, results);
}

jive_tracker_nodestate *
jive_node_get_tracker_state_slow(jive_node * self, jive_tracker_slot slot)
{
	size_t new_size = slot.index + 1;
	
	jive_context * context = self->graph->context;
	self->tracker_slots = jive_context_realloc(context,
		self->tracker_slots, new_size * sizeof(self->tracker_slots[0]));
	
	jive_tracker_nodestate * nodestate;
	size_t n;
	for(n = self->ntracker_slots; n < new_size; n++) {
		nodestate = jive_context_malloc(context, sizeof(*nodestate));
		nodestate->node = self;
		nodestate->cookie = 0;
		nodestate->state = jive_tracker_nodestate_none;
		nodestate->tag = 0;
		self->tracker_slots[n] = nodestate;
	}
	self->ntracker_slots = new_size;
	
	nodestate = self->tracker_slots[slot.index];
	nodestate->cookie = slot.cookie;
	nodestate->state = jive_tracker_nodestate_none;
	nodestate->tag = 0;
	
	return nodestate;
}

/* normal forms */

const jive_node_normal_form_class JIVE_NODE_NORMAL_FORM = {
	parent : 0,
	fini : jive_node_normal_form_fini_,
	normalize_node : jive_node_normal_form_normalize_node_,
	operands_are_normalized : jive_node_normal_form_operands_are_normalized_,
	normalized_create : jive_node_normal_form_normalized_create_,
	set_mutable : jive_node_normal_form_set_mutable_,
	set_cse : jive_node_normal_form_set_cse_
};

void
jive_node_normal_form_fini_(jive_node_normal_form * self)
{
}

bool
jive_node_normal_form_normalize_node_(const jive_node_normal_form * self, jive_node * node)
{
	return true;
}

bool
jive_node_normal_form_operands_are_normalized_(const jive_node_normal_form * self,
	size_t noperands, jive_output * const operands[],
	const jive_node_attrs * attrs)
{
	return true;
}

void
jive_node_normal_form_normalized_create_(const jive_node_normal_form * self, jive_graph * graph,
	const jive_node_attrs * attrs, size_t noperands, jive_output * const operands[],
	jive_output * results[])
{
	const jive_node_class * cls = self->node_class;

	jive_region * region = graph->root_region;
	if (noperands != 0)
		region = jive_region_innermost(noperands, operands);

	jive_node * node = NULL;
	if (self->enable_mutable && self->enable_cse)
		node = jive_node_cse(region, cls, attrs, noperands, operands);

	if (!node)
		node = cls->create(region, attrs, noperands, operands);

	size_t n;
	for (n = 0; n < node->noutputs; n++)
		results[n] = node->outputs[n];
}

void
jive_node_normal_form_set_mutable_(jive_node_normal_form * self, bool enable)
{
	if (self->enable_mutable == enable)
		return;
	
	self->enable_mutable = enable;
	jive_node_normal_form * child;
	JIVE_LIST_ITERATE(self->subclasses, child, normal_form_subclass_list) {
		jive_node_normal_form_set_mutable(child, enable);
	}
	if (enable)
		jive_graph_mark_denormalized(self->graph);
}

void
jive_node_normal_form_set_cse_(jive_node_normal_form * self, bool enable)
{
	if (self->enable_cse == enable)
		return;
	
	self->enable_cse = enable;
	jive_node_normal_form * child;
	JIVE_LIST_ITERATE(self->subclasses, child, normal_form_subclass_list) {
		jive_node_normal_form_set_cse(child, enable);
	}
	if (enable && self->enable_mutable)
		jive_graph_mark_denormalized(self->graph);
}

jive_node *
jive_node_cse_create(const jive_node_normal_form * self, struct jive_region * region,
	const jive_node_attrs * attrs, size_t noperands, jive_output * const operands[])
{
	const jive_node_class * cls = self->node_class;
	
	jive_node * node;
	if (self->enable_mutable && self->enable_cse) {
		node = jive_node_cse(region, cls, attrs, noperands, operands);
		if (node)
			return node;
	}

	return cls->create(region, attrs, noperands, operands);
}

bool
jive_node_normalize(struct jive_node * self)
{
	jive_graph * graph = self->region->graph;
	const jive_node_normal_form * nf = jive_graph_get_nodeclass_form(graph, self->class_);
	return jive_node_normal_form_normalize_node(nf, self);
}
