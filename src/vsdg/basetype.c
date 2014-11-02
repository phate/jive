/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <stdio.h>

#include <jive/util/buffer.h>
#include <jive/util/list.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/gate-interference-private.h>
#include <jive/vsdg/graph-private.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/resource.h>
#include <jive/vsdg/variable.h>

namespace jive {
namespace base {

type::~type() noexcept {}

}
}

void
jive_raise_type_error(const jive::base::type * self, const jive::base::type * other)
{
	throw jive::type_error(self->debug_string(), other->debug_string());
}

jive::input *
jive_type_create_input(const jive::base::type * self, struct jive_node * node, size_t index,
	jive::output * origin)
{
	const jive::base::type * operand_type = &origin->type();

	if (*self != *operand_type)
		jive_raise_type_error(self, operand_type);

	return self->create_input(node, index, origin);
}

/* inputs */

static inline void
jive_input_add_as_user(jive::input * self, jive::output * output)
{
	JIVE_LIST_PUSH_BACK(output->users, self, output_users_list);
	jive_node_add_successor(output->node());
}

static inline void
jive_input_remove_as_user(jive::input * self, jive::output * output)
{
	JIVE_LIST_REMOVE(output->users, self, output_users_list);
	jive_node_remove_successor(self->producer());
}

namespace jive {

input::input(struct jive_node * node_, size_t index_, jive::output * origin)
	: node(node_)
	, index(index_)
	, origin_(origin)
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

input::~input() noexcept
{
	if (ssavar) {
		jive_ssavar_unassign_input(ssavar, this);
		jive_input_unassign_ssavar(this);
	}

	jive_graph_notify_input_destroy(node->graph, this);

	if (gate) {
		JIVE_LIST_REMOVE(gate->inputs, this, gate_inputs_list);

		size_t n;
		for (n = 0; n < node->ninputs; n++) {
			jive::input * other = node->inputs[n];
			if (other == this || !other->gate)
				continue;
			jive_gate_interference_remove(node->graph, gate, other->gate);
		}
	}

	jive_input_remove_as_user(this, origin_);

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
input::swap(jive::input * other) noexcept
{
	JIVE_DEBUG_ASSERT(this->type() == other->type());
	JIVE_DEBUG_ASSERT(this->node == other->node);

	jive_ssavar * v1 = this->ssavar;
	jive_ssavar * v2 = other->ssavar;

	if (v1) jive_ssavar_unassign_input(v1, this);
	if (v2) jive_ssavar_unassign_input(v2, other);

	jive::output * o1 = this->origin();
	jive::output * o2 = other->origin();

	jive_input_remove_as_user(this, o1);
	jive_input_remove_as_user(other, o2);

	jive_input_add_as_user(this, o2);
	jive_input_add_as_user(other, o1);

	this->origin_ = o2;
	other->origin_ = o1;

	if (v2) jive_ssavar_assign_input(v2, this);
	if (v1) jive_ssavar_assign_input(v1, other);

	jive_node_invalidate_depth_from_root(this->node);

	jive_graph_notify_input_change(this->node->graph, this, o1, o2);
	jive_graph_notify_input_change(this->node->graph, other, o2, o1);
}

void
input::divert_origin(jive::output * new_origin) noexcept
{
	JIVE_DEBUG_ASSERT(!this->ssavar);
	internal_divert_origin(new_origin);
}

void
input::internal_divert_origin(jive::output * new_origin) noexcept
{
	const jive::base::type * input_type = &this->type();
	const jive::base::type * operand_type = &new_origin->type();

	if (*input_type != *operand_type) {
		jive_raise_type_error(input_type, operand_type);
	}
	if (dynamic_cast<const jive::achr::type*>(input_type)) {
		throw jive::compiler_error("Type mismatch: Cannot divert edges of 'anchor' type");
	}

	JIVE_DEBUG_ASSERT(this->node->graph == new_origin->node()->graph);

	if (this->producer()->region != this->node->region)
		jive_region_hull_remove_input(this->node->region, this);

	if (this->node->graph->floating_region_count)
		jive_region_check_move_floating(this->node->region, new_origin->node()->region);

	JIVE_DEBUG_ASSERT(jive_node_valid_edge(this->node, new_origin));

	jive::output * old_origin = this->origin();

	jive_input_remove_as_user(this, old_origin);
	this->origin_ = new_origin;
	jive_input_add_as_user(this, new_origin);

	if (new_origin->node()->region != this->node->region)
		jive_region_hull_add_input(this->node->region, this);

	jive_node_invalidate_depth_from_root(this->node);

	jive_graph_mark_denormalized(new_origin->node()->graph);

	jive_graph_notify_input_change(this->node->graph, this, old_origin, new_origin);

#ifdef JIVE_DEBUG
	jive_region_verify_hull(this->node->region->graph->root_region);
#endif
}

void
input::label(jive_buffer & buffer) const
{
	if (gate) {
		gate->label(buffer);
	} else {
		char tmp[16];
		snprintf(tmp, sizeof(tmp), "#%zd", index);
		jive_buffer_putstr(&buffer, tmp);
	}
}

}	//jive namespace

jive_variable *
jive_input_get_constraint(const jive::input * self)
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
jive_input_unassign_ssavar(jive::input * self)
{
	if (self->ssavar) jive_ssavar_unassign_input(self->ssavar, self);
}

jive_ssavar *
jive_input_auto_assign_variable(jive::input * self)
{
	if (self->ssavar)
		return self->ssavar;
	
	jive_ssavar * ssavar;
	if (self->origin()->ssavar) {
		ssavar = self->origin()->ssavar;
		jive_variable_merge(ssavar->variable, jive_input_get_constraint(self));
	} else {
		ssavar = jive_ssavar_create(self->origin(), jive_input_get_constraint(self));
	}
	
	jive_ssavar_assign_input(ssavar, self);
	return ssavar;
}

jive_ssavar *
jive_input_auto_merge_variable(jive::input * self)
{
	if (self->ssavar)
		return self->ssavar;
	
	jive_ssavar * ssavar = NULL;
	
	if (self->origin()->ssavar) {
		ssavar = self->origin()->ssavar;
	} else {
		jive::input * user;
		JIVE_LIST_ITERATE(self->origin()->users, user, output_users_list) {
			if (user->ssavar) {
				ssavar = user->ssavar;
				break;
			}
		}
	}
	
	if (!ssavar)
		ssavar = jive_ssavar_create(self->origin(), jive_input_get_constraint(self));
	
	jive_variable_merge(ssavar->variable, jive_input_get_constraint(self));
	jive_ssavar_assign_input(ssavar, self);
	return ssavar;
}

/* outputs */

namespace jive {

output::output(jive_node * node, size_t index_)
	: node_(node)
	, index(index_)
	, gate(nullptr)
	, ssavar(nullptr)
	, required_rescls(&jive_root_resource_class)
{
	users.first = users.last = nullptr;
	originating_ssavars.first = originating_ssavars.last = nullptr;
	gate_outputs_list.prev = gate_outputs_list.next = nullptr;
}

output::~output() noexcept
{
	JIVE_DEBUG_ASSERT(users.first == nullptr && users.last == nullptr);
	
	jive_graph_notify_output_destroy(node_->graph, this);

	if (ssavar)
		jive_ssavar_unassign_output(ssavar, this);
		
	if (gate) {
		JIVE_LIST_REMOVE(gate->outputs, this, gate_outputs_list);
		
		size_t n;
		for (n = 0; n < node_->noutputs; n++) {
			jive::output * other = node_->outputs[n];
			if (other == this || !other->gate)
				continue;
			jive_gate_interference_remove(node_->graph, gate, other->gate);
		}
	}
	
	node_->noutputs--;
	size_t n;
	for (n = index; n < node_->noutputs; n++) {
		node_->outputs[n] = node_->outputs[n+1];
		node_->outputs[n]->index = n;
	}
	
	JIVE_DEBUG_ASSERT(originating_ssavars.first == 0);
}

void
output::label(jive_buffer & buffer) const
{
	if (gate) {
		gate->label(buffer);
	} else {
		char tmp[16];
		snprintf(tmp, sizeof(tmp), "#%zd", index);
		jive_buffer_putstr(&buffer, tmp);
	}
}

}	//jive namespace

jive_variable *
jive_output_get_constraint(const jive::output * self)
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
	variable = jive_variable_create(self->node()->graph);
	jive_variable_set_resource_class(variable, self->required_rescls);
	return variable;
}

void
jive_output_unassign_ssavar(jive::output * self)
{
	if (self->ssavar) jive_ssavar_unassign_output(self->ssavar, self);
}

jive_ssavar *
jive_output_auto_assign_variable(jive::output * self)
{
	if (self->ssavar == 0) {
		jive_ssavar * ssavar = 0;
		jive::input * user;
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
jive_output_auto_merge_variable(jive::output * self)
{
	if (!self->ssavar) {
		jive_variable * variable = jive_output_get_constraint(self);
		jive_ssavar * ssavar = NULL;
		jive::input * user;
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
	
	jive::input * user;
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
jive_output_replace(jive::output * self, jive::output * other)
{
	while(self->users.first) {
		jive::input * input = self->users.first;
		input->divert_origin(other);
	}
}

/* gates */

namespace jive {

gate::gate(jive_graph * graph, const char name_[])
	: graph (graph)
{
	name = name_;
	inputs.first = inputs.last = nullptr;
	outputs.first = outputs.last = nullptr;
	may_spill = true;
	variable = nullptr;
	variable_gate_list.prev = variable_gate_list.next = nullptr;
	graph_gate_list.prev = graph_gate_list.next = nullptr;
	required_rescls = &jive_root_resource_class;
	
	JIVE_LIST_PUSH_BACK(graph->gates, this, graph_gate_list);
}

gate::~gate() noexcept
{
	JIVE_DEBUG_ASSERT(inputs.first == nullptr && inputs.last == nullptr);
	JIVE_DEBUG_ASSERT(outputs.first == nullptr && outputs.last == nullptr);
	
	if (variable)
		jive_variable_unassign_gate(variable, this);

	JIVE_LIST_REMOVE(graph->gates, this, graph_gate_list);
}

void
gate::label(jive_buffer & buffer) const
{
	jive_buffer_putstr(&buffer, name.c_str());
}

jive::input *
gate::create_input(jive_node * node, size_t index, jive::output * origin)
{
	jive::input * input = jive_type_create_input(&type(), node, index, origin);
	input->required_rescls = required_rescls;
	input->gate = this;
	JIVE_LIST_PUSH_BACK(this->inputs, input, gate_inputs_list);
	return input;
}

jive::output *
gate::create_output(jive_node * node, size_t index)
{
	jive::output * output = type().create_output(node, index);
	output->required_rescls = required_rescls;
	output->gate = this;
	JIVE_LIST_PUSH_BACK(this->outputs, output, gate_outputs_list);
	return output;
}

}	//jive namespace

jive_variable *
jive_gate_get_constraint(jive::gate * self)
{
	if (self->variable) return self->variable;
	
	jive_variable * variable = jive_variable_create(self->graph);
	jive_variable_set_resource_class(variable, self->required_rescls);
	
	return variable;
}

size_t
jive_gate_interferes_with(const jive::gate * self, const jive::gate * other)
{
	auto i = self->interference.find(other);
	if (i != self->interference.end())
		return i->whole->count;

	return 0;
}

void
jive_gate_merge(jive::gate * self, jive::gate * other)
{
	jive::input * input, * input_next;
	JIVE_LIST_ITERATE_SAFE(other->inputs, input, input_next, gate_inputs_list) {
		size_t n;
		for(n = 0; n<input->node->ninputs; n++) {
			jive::input * other_input = input->node->inputs[n];
			if (other_input == input) continue;
			if (other_input->gate == 0) continue;
			jive_gate_interference_remove(self->graph, other, other_input->gate);
			jive_gate_interference_add(self->graph, self, other_input->gate);
		}
		JIVE_LIST_REMOVE(other->inputs, input, gate_inputs_list);
		input->gate = self;
		JIVE_LIST_PUSH_BACK(other->inputs, input, gate_inputs_list);
	}
	
	jive::output * output, * output_next;
	JIVE_LIST_ITERATE_SAFE(other->outputs, output, output_next, gate_outputs_list) {
		size_t n;
		for(n = 0; n < output->node()->noutputs; n++) {
			jive::output * other_output = output->node()->outputs[n];
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
	
	self->name.append("_").append(other->name);
}

void
jive_gate_split(jive::gate * self)
{
	/* split off this gate from others assigned to the same variable */
	jive_variable * new_variable = jive_variable_create(self->variable->graph);
	jive_variable_set_resource_class(new_variable, jive_variable_get_resource_class(self->variable));
	jive_variable_set_resource_name(new_variable, jive_variable_get_resource_name(self->variable));
			
	jive_variable_unassign_gate(self->variable, self);
	jive_variable_assign_gate(new_variable, self);
}
