/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <stdio.h>

#include <jive/util/buffer.h>
#include <jive/util/list.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/gate-interference-private.h>
#include <jive/vsdg/graph-private.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/resource.h>
#include <jive/vsdg/variable.h>

jive_type::~jive_type() noexcept {}

jive_type::jive_type(const jive_type_class * class__) noexcept
	: class_(class__)
{}

void
jive_type_fini_(jive_type * self)
{
}

void
jive_type_get_label_(const jive_type * self, struct jive_buffer * buffer)
{
	jive_buffer_putstr(buffer, self->class_->name);
}

jive_input *
jive_type_create_input_(const jive_type * self, struct jive_node * node, size_t index,
	jive_output * initial_operand)
{
	return nullptr;
}

jive_output *
jive_type_create_output_(const jive_type * self, struct jive_node * node, size_t index)
{
	return nullptr;
}

jive_gate *
jive_type_create_gate_(const jive_type * self, struct jive_graph * graph, const char * name)
{
	return nullptr;
}

bool
jive_type_equals_(const jive_type * self, const jive_type * other)
{
	return self->class_ == other->class_;
}

jive_type *
jive_type_copy_(const jive_type * self)
{
	/* base-type non copyable */
	return NULL;
}

void
jive_raise_type_error(const jive_type * self, const jive_type * other, jive_context * context)
{
	jive_buffer input_type_buffer, operand_type_buffer;
	jive_buffer_init(&input_type_buffer, context);
	jive_buffer_init(&operand_type_buffer, context);
	jive_type_get_label(self, &input_type_buffer);
	jive_type_get_label(other, &operand_type_buffer);
	
	char * error_message = jive_context_strjoin(context,
		"Type mismatch: required '", jive_buffer_to_string(&input_type_buffer),
		"' got '", jive_buffer_to_string(&operand_type_buffer), "'", NULL);

	jive_buffer_fini(&input_type_buffer);
	jive_buffer_fini(&operand_type_buffer);
	jive_context_fatal_error(context, error_message);
}

struct jive_input *
jive_type_create_input(const jive_type * self, struct jive_node * node, size_t index,
	jive_output * initial_operand)
{
	const jive_type * operand_type = jive_output_get_type(initial_operand);

	if (!jive_type_equals(self, operand_type))
		jive_raise_type_error(self, operand_type, node->graph->context);

	return self->class_->create_input(self, node, index, initial_operand);
}

const jive_type_class JIVE_TYPE = {
	parent : 0,
	name : "X",
	fini : jive_type_fini_,
	get_label : jive_type_get_label_,
	create_input : jive_type_create_input_,
	create_output : jive_type_create_output_,
	create_gate : jive_type_create_gate_,
	equals : jive_type_equals_,
	copy : jive_type_copy_
};

void
jive_type_destroy(struct jive_type * self)
{
	jive_type_fini(self);
	delete self;
}

/* inputs */

static inline void
jive_input_add_as_user(jive_input * self, jive_output * output)
{
	JIVE_LIST_PUSH_BACK(output->users, self, output_users_list);
	jive_node_add_successor(output->node);
}

static inline void
jive_input_remove_as_user(jive_input * self, jive_output * output)
{
	JIVE_LIST_REMOVE(output->users, self, output_users_list);
	jive_node_remove_successor(self->origin->node);
}

jive_input::jive_input(struct jive_node * node_, size_t index_, jive_output * origin_)
	: node(node_)
	, index(index_)
	, origin(origin_)
	, gate(nullptr)
	, required_rescls(&jive_root_resource_class)
	, ssavar(nullptr)
{
	output_users_list.prev = output_users_list.next = nullptr;
	gate_inputs_list.prev = gate_inputs_list.next = nullptr;
	ssavar_input_list.prev = ssavar_input_list.next = nullptr;
	hull.first = hull.last = nullptr;

	jive_input_add_as_user(this, origin);
	jive_region_hull_add_input(node->region, this);
}

jive_input::~jive_input() noexcept
{
	if (ssavar)
		jive_input_unassign_ssavar(this);

	if (gate) {
		JIVE_LIST_REMOVE(gate->inputs, this, gate_inputs_list);

		size_t n;
		for (n = 0; n < node->ninputs; n++) {
			jive_input * other = node->inputs[n];
			if (other == this || !other->gate)
				continue;
			jive_gate_interference_remove(node->graph, gate, other->gate);
		}
	}

	jive_input_remove_as_user(this, origin);

	size_t n;
	node->ninputs--;
	for (n = index; n < node->ninputs; n++) {
		node->inputs[n] = node->inputs[n+1];
		node->inputs[n]->index = n;
	}
	if (node->ninputs == 0)
		JIVE_LIST_PUSH_BACK(node->region->top_nodes, node, region_top_node_list);

	jive_region_hull_remove_input(node->region, this);
}

void
jive_input::label(jive_buffer & buffer) const
{
	if (gate) {
		jive_gate_get_label(gate, &buffer);
	} else {
		char tmp[16];
		snprintf(tmp, sizeof(tmp), "#%zd", index);
		jive_buffer_putstr(&buffer, tmp);
	}
}

jive_variable *
jive_input_get_constraint(const jive_input * self)
{
	jive_variable * variable;
	if (self->gate) {
		variable = self->gate->variable;
		if (!variable) {
			variable = jive_gate_get_constraint(self->gate);
			jive_variable_assign_gate(variable, self->gate);
		}
		return variable;
	}
	variable = jive_variable_create(self->node->graph);
	jive_variable_set_resource_class(variable, self->required_rescls);
	return variable;
}

void
jive_input_unassign_ssavar(jive_input * self)
{
	if (self->ssavar) jive_ssavar_unassign_input(self->ssavar, self);
}

void
jive_input_divert_origin(jive_input * self, jive_output * new_origin)
{
	JIVE_DEBUG_ASSERT(!self->ssavar);
	jive_input_internal_divert_origin(self, new_origin);
}

void
jive_input_internal_divert_origin(jive_input * self, jive_output * new_origin)
{
	const jive_type * input_type = jive_input_get_type(self);
	const jive_type * operand_type = jive_output_get_type(new_origin);
	
	if (!jive_type_equals(input_type, operand_type)) {
		jive_raise_type_error(input_type, operand_type, self->node->graph->context);
	}
	if (input_type->class_ == &JIVE_ANCHOR_TYPE) {
		jive_context_fatal_error(self->node->graph->context,
			"Type mismatch: Cannot divert edges of 'anchor' type");
	}
	
	JIVE_DEBUG_ASSERT(self->node->graph == new_origin->node->graph);

	if (self->origin->node->region != self->node->region)
		jive_region_hull_remove_input(self->node->region, self);

	if (self->node->graph->floating_region_count)
		jive_region_check_move_floating(self->node->region, new_origin->node->region);
	
	JIVE_DEBUG_ASSERT(jive_node_valid_edge(self->node, new_origin));
	
	jive_output * old_origin = self->origin;
	
	jive_input_remove_as_user(self, old_origin);
	self->origin = new_origin;
	jive_input_add_as_user(self, new_origin);
	
	if (new_origin->node->region != self->node->region)
		jive_region_hull_add_input(self->node->region, self);

	jive_node_invalidate_depth_from_root(self->node);
	
	jive_graph_mark_denormalized(new_origin->node->graph);
	
	jive_graph_notify_input_change(self->node->graph, self, old_origin, new_origin);

#ifdef JIVE_DEBUG
	jive_region_verify_hull(self->node->region->graph->root_region);
#endif
}

void
jive_input_swap(jive_input * self, jive_input * other)
{
	JIVE_DEBUG_ASSERT(jive_type_equals(jive_input_get_type(self), jive_input_get_type(other)));
	JIVE_DEBUG_ASSERT(self->node == other->node);
	
	jive_ssavar * v1 = self->ssavar;
	jive_ssavar * v2 = other->ssavar;
	
	if (v1) jive_ssavar_unassign_input(v1, self);
	if (v2) jive_ssavar_unassign_input(v2, other);
	
	jive_output * o1 = self->origin;
	jive_output * o2 = other->origin;
	
	jive_input_remove_as_user(self, o1);
	jive_input_remove_as_user(other, o2);
	
	jive_input_add_as_user(self, o2);
	jive_input_add_as_user(other, o1);
	
	self->origin = o2;
	other->origin = o1;
	
	if (v2) jive_ssavar_assign_input(v2, self);
	if (v1) jive_ssavar_assign_input(v1, other);
	
	jive_node_invalidate_depth_from_root(self->node);
	
	jive_graph_notify_input_change(self->node->graph, self, o1, o2);
	jive_graph_notify_input_change(self->node->graph, other, o2, o1);
}

jive_ssavar *
jive_input_auto_assign_variable(jive_input * self)
{
	if (self->ssavar)
		return self->ssavar;
	
	jive_ssavar * ssavar;
	if (self->origin->ssavar) {
		ssavar = self->origin->ssavar;
		jive_variable_merge(ssavar->variable, jive_input_get_constraint(self));
	} else {
		ssavar = jive_ssavar_create(self->origin, jive_input_get_constraint(self));
	}
	
	jive_ssavar_assign_input(ssavar, self);
	return ssavar;
}

jive_ssavar *
jive_input_auto_merge_variable(jive_input * self)
{
	if (self->ssavar)
		return self->ssavar;
	
	jive_ssavar * ssavar = NULL;
	
	if (self->origin->ssavar) {
		ssavar = self->origin->ssavar;
	} else {
		jive_input * user;
		JIVE_LIST_ITERATE(self->origin->users, user, output_users_list) {
			if (user->ssavar) {
				ssavar = user->ssavar;
				break;
			}
		}
	}
	
	if (!ssavar)
		ssavar = jive_ssavar_create(self->origin, jive_input_get_constraint(self));
	
	jive_variable_merge(ssavar->variable, jive_input_get_constraint(self));
	jive_ssavar_assign_input(ssavar, self);
	return ssavar;
}

void
jive_input_destroy(jive_input * self)
{
	if (self->ssavar) jive_ssavar_unassign_input(self->ssavar, self);
	if (self->node->region) jive_graph_notify_input_destroy(self->node->graph, self);
	
	delete self;
}

/* outputs */

jive_output::jive_output(struct jive_node * node_, size_t index_)
	: node(node_)
	, index(index_)
	, gate(nullptr)
	, ssavar(nullptr)
	, required_rescls(&jive_root_resource_class)
{
	users.first = users.last = nullptr;
	originating_ssavars.first = originating_ssavars.last = nullptr;
	gate_outputs_list.prev = gate_outputs_list.next = nullptr;
}

jive_output::~jive_output() noexcept
{
	JIVE_DEBUG_ASSERT(users.first == nullptr && users.last == nullptr);
	
	if (ssavar)
		jive_ssavar_unassign_output(ssavar, this);
		
	if (gate) {
		JIVE_LIST_REMOVE(gate->outputs, this, gate_outputs_list);
		
		size_t n;
		for (n = 0; n < node->noutputs; n++) {
			jive_output * other = node->outputs[n];
			if (other == this || !other->gate)
				continue;
			jive_gate_interference_remove(node->graph, gate, other->gate);
		}
	}
	
	node->noutputs--;
	size_t n;
	for (n = index; n < node->noutputs; n++) {
		node->outputs[n] = node->outputs[n+1];
		node->outputs[n]->index = n;
	}
	
	JIVE_DEBUG_ASSERT(originating_ssavars.first == 0);
}

void
jive_output::label(jive_buffer & buffer) const
{
	if (gate) {
		jive_gate_get_label(gate, &buffer);
	} else {
		char tmp[16];
		snprintf(tmp, sizeof(tmp), "#%zd", index);
		jive_buffer_putstr(&buffer, tmp);
	}
}

jive_variable *
jive_output_get_constraint(const jive_output * self)
{
	jive_variable * variable;
	if (self->gate) {
		variable = self->gate->variable;
		if (!variable) {
			variable = jive_gate_get_constraint(self->gate);
			jive_variable_assign_gate(variable, self->gate);
		}
		return variable;
	}
	variable = jive_variable_create(self->node->graph);
	jive_variable_set_resource_class(variable, self->required_rescls);
	return variable;
}

void
jive_output_unassign_ssavar(jive_output * self)
{
	if (self->ssavar) jive_ssavar_unassign_output(self->ssavar, self);
}

jive_ssavar *
jive_output_auto_assign_variable(jive_output * self)
{
	if (self->ssavar == 0) {
		jive_ssavar * ssavar = 0;
		jive_input * user;
		JIVE_LIST_ITERATE(self->users, user, output_users_list) {
			if (!user->ssavar) continue;
			if (ssavar) {
				jive_variable_merge(ssavar->variable, user->ssavar->variable);
				jive_ssavar_merge(ssavar, user->ssavar);
			} else
				ssavar = user->ssavar;
		}
		
		if (ssavar) {
			jive_variable_merge(ssavar->variable, jive_output_get_constraint(self));
		} else {
			ssavar = jive_ssavar_create(self, jive_output_get_constraint(self));
		}
		
		jive_ssavar_assign_output(ssavar, self);
	}
	
	return self->ssavar;
}

jive_ssavar *
jive_output_auto_merge_variable(jive_output * self)
{
	if (!self->ssavar) {
		jive_variable * variable = jive_output_get_constraint(self);
		jive_ssavar * ssavar = NULL;
		jive_input * user;
		JIVE_LIST_ITERATE(self->users, user, output_users_list) {
			if (user->ssavar) {
				jive_variable_merge(user->ssavar->variable, variable);
				JIVE_DEBUG_ASSERT( ssavar == NULL || ssavar == user->ssavar );
				ssavar = user->ssavar;
				variable = user->ssavar->variable;
			}
		}
		if (!ssavar) ssavar = jive_ssavar_create(self, variable);
		jive_ssavar_assign_output(ssavar, self);
	}
	
	jive_input * user;
	JIVE_LIST_ITERATE(self->users, user, output_users_list) {
		if (!user->ssavar) {
			jive_variable_merge(self->ssavar->variable, jive_input_get_constraint(user));
			jive_ssavar_assign_input(self->ssavar, user);
		} else if (user->ssavar != self->ssavar) {
			/* FIXME: maybe better to merge ssavar? */
			jive_variable_merge(self->ssavar->variable, user->ssavar->variable);
			jive_input_unassign_ssavar(user);
			jive_ssavar_assign_input(self->ssavar, user);
		}
	}
	return self->ssavar;
}

void
jive_output_replace(jive_output * self, jive_output * other)
{
	while(self->users.first) {
		jive_input * input = self->users.first;
		jive_input_divert_origin(input, other);
	}
}

void
jive_output_destroy(jive_output * self)
{
	if (self->node->region) jive_graph_notify_output_destroy(self->node->graph, self);
	
	delete self;
}

/* gates */

jive_gate::jive_gate(jive_graph * graph, const char name_[])
	: graph (graph)
{
	name = jive_context_strdup(graph->context, name_);
	inputs.first = inputs.last = nullptr;
	outputs.first = outputs.last = nullptr;
	may_spill = true;
	variable = nullptr;
	jive_gate_interference_hash_init(&interference, graph->context);
	variable_gate_list.prev = variable_gate_list.next = nullptr;
	graph_gate_list.prev = graph_gate_list.next = nullptr;
	required_rescls = &jive_root_resource_class;
	
	JIVE_LIST_PUSH_BACK(graph->gates, this, graph_gate_list);
}

jive_gate::~jive_gate() noexcept
{
	JIVE_DEBUG_ASSERT(inputs.first == nullptr && inputs.last == nullptr);
	JIVE_DEBUG_ASSERT(outputs.first == nullptr && outputs.last == nullptr);
	
	if (variable)
		jive_variable_unassign_gate(variable, this);
	
	jive_gate_interference_hash_fini(&interference);
	jive_context_free(graph->context, name);
	
	JIVE_LIST_REMOVE(graph->gates, this, graph_gate_list);
}

void
jive_gate::label(jive_buffer & buffer) const
{
	jive_buffer_putstr(&buffer, name);
}

jive_variable *
jive_gate_get_constraint(jive_gate * self)
{
	if (self->variable) return self->variable;
	
	jive_variable * variable = jive_variable_create(self->graph);
	jive_variable_set_resource_class(variable, self->required_rescls);
	
	return variable;
}

size_t
jive_gate_interferes_with(const jive_gate * self, const jive_gate * other)
{
	jive_gate_interference_part * part;
	part = jive_gate_interference_hash_lookup(&self->interference, other);
	if (part) return part->whole->count;
	else return 0;
}

void
jive_gate_destroy(jive_gate * self)
{
	delete self;
}

void
jive_gate_merge(jive_gate * self, jive_gate * other)
{
	jive_input * input, * input_next;
	JIVE_LIST_ITERATE_SAFE(other->inputs, input, input_next, gate_inputs_list) {
		size_t n;
		for(n = 0; n<input->node->ninputs; n++) {
			jive_input * other_input = input->node->inputs[n];
			if (other_input == input) continue;
			if (other_input->gate == 0) continue;
			jive_gate_interference_remove(self->graph, other, other_input->gate);
			jive_gate_interference_add(self->graph, self, other_input->gate);
		}
		JIVE_LIST_REMOVE(other->inputs, input, gate_inputs_list);
		input->gate = self;
		JIVE_LIST_PUSH_BACK(other->inputs, input, gate_inputs_list);
	}
	
	jive_output * output, * output_next;
	JIVE_LIST_ITERATE_SAFE(other->outputs, output, output_next, gate_outputs_list) {
		size_t n;
		for(n = 0; n<output->node->noutputs; n++) {
			jive_output * other_output = output->node->outputs[n];
			if (other_output == output) continue;
			if (other_output->gate == 0) continue;
			jive_gate_interference_remove(self->graph, other, other_output->gate);
			jive_gate_interference_add(self->graph, self, other_output->gate);
		}
		JIVE_LIST_REMOVE(other->outputs, output, gate_outputs_list);
		output->gate = self;
		JIVE_LIST_PUSH_BACK(other->outputs, output, gate_outputs_list);
	}
	
	if (self->variable)
		jive_variable_merge(self->variable, other->variable);
	
	jive_context * context = self->graph->context;
	char * name = jive_context_strjoin(context, self->name, "_", other->name, NULL);
	jive_context_free(context, self->name);
	self->name = name;
}

void
jive_gate_split(jive_gate * self)
{
	/* split off this gate from others assigned to the same variable */
	jive_variable * new_variable = jive_variable_create(self->variable->graph);
	jive_variable_set_resource_class(new_variable, jive_variable_get_resource_class(self->variable));
	jive_variable_set_resource_name(new_variable, jive_variable_get_resource_name(self->variable));
			
	jive_variable_unassign_gate(self->variable, self);
	jive_variable_assign_gate(new_variable, self);
}
