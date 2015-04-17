/*
 * Copyright 2010 2011 2012 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
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
#include <jive/vsdg/resource.h>
#include <jive/vsdg/substitution.h>
#include <jive/vsdg/variable.h>

static void
jive_node_invalidate_depth_from_root(jive_node * self)
{
	size_t new_depth_from_root = 0;
	for (size_t n = 0; n < self->ninputs; n++)
		new_depth_from_root = std::max(self->producer(n)->depth_from_root + 1, new_depth_from_root);

	size_t old_depth_from_root = self->depth_from_root;
	if (old_depth_from_root == new_depth_from_root)
		return;
	self->depth_from_root = new_depth_from_root;

	self->graph->on_node_depth_change(self, old_depth_from_root);

	for (size_t n = 0; n < self->noutputs; n++) {
		jive::input * user = self->outputs[n]->users.first;
		while (user) {
			jive_node_invalidate_depth_from_root(user->node());
			user = user->output_users_list.next;
		}
	}
}

/* inputs */

static inline void
jive_input_add_as_user(jive::input * self, jive::output * output)
{
	JIVE_LIST_PUSH_BACK(output->users, self, output_users_list);

	if (unlikely(output->node()->nsuccessors == 0)) {
		JIVE_LIST_REMOVE(output->node()->graph->bottom, output->node(), graph_bottom_list);
	}
	output->node()->nsuccessors ++;
}

static inline void
jive_input_remove_as_user(jive::input * self, jive::output * output)
{
	JIVE_LIST_REMOVE(output->users, self, output_users_list);

	self->producer()->nsuccessors --;
	if (unlikely(self->producer()->nsuccessors == 0)) {
		JIVE_LIST_PUSH_BACK(self->producer()->graph->bottom, self->producer(), graph_bottom_list);
	}
}

namespace jive {

input::input(
	struct jive_node * node,
	size_t index,
	jive::output * origin,
	const jive::base::type & type)
	: gate(nullptr)
	, ssavar(nullptr)
	, required_rescls(&jive_root_resource_class)
	, index_(index)
	, origin_(origin)
	, node_(node)
	, type_(type.copy())
{
	if (type != origin->type())
		throw jive::type_error(type.debug_string(), origin->type().debug_string());

	output_users_list.prev = output_users_list.next = nullptr;
	gate_inputs_list.prev = gate_inputs_list.next = nullptr;
	ssavar_input_list.prev = ssavar_input_list.next = nullptr;
	hull.first = hull.last = nullptr;

	jive_input_add_as_user(this, origin);
	jive_region_hull_add_input(node->region, this);

	/*
		FIXME: This is going to be removed once we switched Jive to the new node representation.
	*/
	if (dynamic_cast<const jive::achr::type*>(&type)) {
		JIVE_DEBUG_ASSERT(origin->node()->region->anchor == nullptr);
		origin->node()->region->anchor = this;
	}
}

input::~input() noexcept
{
	if (ssavar) {
		jive_ssavar_unassign_input(ssavar, this);
		jive_input_unassign_ssavar(this);
	}

	node()->graph->on_input_destroy(this);

	if (gate) {
		JIVE_LIST_REMOVE(gate->inputs, this, gate_inputs_list);

		for (size_t n = 0; n < node()->ninputs; n++) {
			jive::input * other = node()->inputs[n];
			if (other == this || !other->gate)
				continue;
			jive_gate_interference_remove(node()->graph, gate, other->gate);
		}
	}

	jive_input_remove_as_user(this, origin_);

	node()->ninputs--;
	for (size_t n = index(); n < node()->ninputs; n++) {
		node()->inputs[n] = node()->inputs[n+1];
		node()->inputs[n]->index_ = n;
	}
	if (node()->ninputs == 0)
		JIVE_LIST_PUSH_BACK(node()->region->top_nodes, node_, region_top_node_list);

	jive_region_hull_remove_input(node()->region, this);
}

void
input::swap(jive::input * other) noexcept
{
	JIVE_DEBUG_ASSERT(this->type() == other->type());
	JIVE_DEBUG_ASSERT(this->node() == other->node());

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

	jive_node_invalidate_depth_from_root(this->node());

	node()->graph->on_input_change(this, o1, o2);
	node()->graph->on_input_change(other, o2, o1);
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

	if (*input_type != *operand_type)
		throw jive::type_error(input_type->debug_string(), operand_type->debug_string());

	if (dynamic_cast<const jive::achr::type*>(input_type)) {
		throw jive::compiler_error("Type mismatch: Cannot divert edges of 'anchor' type");
	}

	JIVE_DEBUG_ASSERT(this->node()->graph == new_origin->node()->graph);

	if (this->producer()->region != this->node()->region)
		jive_region_hull_remove_input(this->node()->region, this);

	JIVE_DEBUG_ASSERT(jive_node_valid_edge(this->node(), new_origin));

	jive::output * old_origin = this->origin();

	jive_input_remove_as_user(this, old_origin);
	this->origin_ = new_origin;
	jive_input_add_as_user(this, new_origin);

	if (new_origin->node()->region != this->node()->region)
		jive_region_hull_add_input(this->node()->region, this);

	jive_node_invalidate_depth_from_root(this->node());

	jive_graph_mark_denormalized(new_origin->node()->graph);

	node()->graph->on_input_change(this, old_origin, new_origin);

#ifdef JIVE_DEBUG
	jive_region_verify_hull(this->node()->region->graph->root_region);
#endif
}

}	//jive namespace

jive_variable *
jive_input_get_constraint(const jive::input * self)
{
	jive_variable * variable;
	if (self->gate) {
		variable = self->gate->variable;
		if (!variable) {
			variable = jive_variable_create(self->gate->graph());
			jive_variable_set_resource_class(variable, self->gate->required_rescls);
			jive_variable_assign_gate(variable, self->gate);
		}
		return variable;
	}
	variable = jive_variable_create(self->node()->graph);
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

output::output(jive_node * node, size_t index, const jive::base::type & type)
	: gate(nullptr)
	, ssavar(nullptr)
	, required_rescls(&jive_root_resource_class)
	, index_(index)
	, node_(node)
	, type_(type.copy())
{
	users.first = users.last = nullptr;
	originating_ssavars.first = originating_ssavars.last = nullptr;
	gate_outputs_list.prev = gate_outputs_list.next = nullptr;
}

output::~output() noexcept
{
	JIVE_DEBUG_ASSERT(users.first == nullptr && users.last == nullptr);

	node_->graph->on_output_destroy(this);

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
	for (n = index(); n < node_->noutputs; n++) {
		node_->outputs[n] = node_->outputs[n+1];
		node_->outputs[n]->index_ = n;
	}

	JIVE_DEBUG_ASSERT(originating_ssavars.first == 0);
}

}	//jive namespace

jive_variable *
jive_output_get_constraint(const jive::output * self)
{
	jive_variable * variable;
	if (self->gate) {
		variable = self->gate->variable;
		if (!variable) {
			variable = jive_variable_create(self->gate->graph());
			jive_variable_set_resource_class(variable, self->required_rescls);
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

gate::gate(jive_graph * graph, const char name_[], const jive::base::type & type)
	: graph_(graph)
	, type_(type.copy())
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

	JIVE_LIST_REMOVE(graph()->gates, this, graph_gate_list);
}

}	//jive namespace

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

static void
jive_uninitialized_node_add_output_(jive_node * self, jive::output * output)
{
	JIVE_DEBUG_ASSERT(!self->graph->resources_fully_assigned);
	
	self->noutputs ++;
	self->outputs.resize(self->noutputs);
	self->outputs[output->index()] = output;
}

static jive::output *
jive_uninitialized_node_add_output(jive_node * self, const jive::base::type * type)
{
	jive::output * output = new jive::output(self, self->noutputs, *type);
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
	self->inputs[input->index()] = input;

}

static jive::input *
jive_uninitialized_node_add_input(jive_node * self, const jive::base::type * type,
	jive::output * initial_operand)
{
	jive::input * input = new jive::input(self, self->ninputs, initial_operand, *type);
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
	
	self->graph->on_node_create(self);
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
	self->graph->on_input_create(input);
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
	jive::input * input = new jive::input(self, self->ninputs, initial_operand, *type);
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
	
	if (self->region) {
		self->graph->on_output_create(output);
	}
}

jive::output *
jive_node_add_output(jive_node * self, const jive::base::type * type)
{
	jive::output * output = new jive::output(self, self->noutputs, *type);
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
	jive::input * input = new jive::input(self, self->ninputs, initial_operand, gate->type());
	input->required_rescls = gate->required_rescls;
	input->gate = gate;
	JIVE_LIST_PUSH_BACK(gate->inputs, input, gate_inputs_list);

	for (size_t n = 0; n < input->index(); n++) {
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
	jive::output * output = new jive::output(self, self->noutputs, gate->type());
	output->required_rescls = gate->required_rescls;
	output->gate = gate;
	JIVE_LIST_PUSH_BACK(gate->outputs, output, gate_outputs_list);

	for (size_t n = 0; n < output->index(); n++) {
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
	use_count->clear();
	
	for (size_t n = 0; n < self->ninputs; n++) {
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
		
		use_count->add(rescls);
	}
}

void
jive_node_get_use_count_output(const jive_node * self, jive_resource_class_count * use_count)
{
	use_count->clear();
	
	for (size_t n = 0; n < self->noutputs; n++) {
		jive::output * output = self->outputs[n];
		
		const jive_resource_class * rescls;
		if (output->ssavar) rescls = output->ssavar->variable->rescls;
		else if (output->gate) rescls = output->gate->required_rescls;
		else rescls = output->required_rescls;
		
		use_count->add(rescls);
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
			jive_region * region = user->node()->region;
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
			jive_region_hull_remove_input(user->node()->region, user);
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
			if (self->region != user->node()->region)
				jive_region_hull_add_input(user->node()->region, user);
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
jive_node_copy_substitute(
	const jive_node * self,
	jive_region * target,
	jive::substitution_map & substitution)
{
	std::vector<jive::output*> operands(self->noperands);
	for (size_t n = 0; n < self->noperands; n++) {
		operands[n] = substitution.lookup(self->inputs[n]->origin());
		if (!operands[n]) {
			operands[n] = self->inputs[n]->origin();
		}
	}
	
	jive_node * new_node = jive_node_copy(self, target, &operands[0]);
	for (size_t n = self->noperands; n < self->ninputs; n++) {
		jive::output * origin = substitution.lookup(self->inputs[n]->origin());
		if (!origin) {
			origin =  self->inputs[n]->origin();
		}

		if (self->inputs[n]->gate) {
			jive::gate * gate = self->inputs[n]->gate;

			jive::gate * target_gate = substitution.lookup(gate);
			if (!target_gate) {
				target_gate = jive_graph_create_gate(target->graph, gate->name, gate->type());
				target_gate->required_rescls = gate->required_rescls;
				substitution.insert(gate, target_gate);
			}
		} else {
			jive::input * input = jive_node_add_input(new_node, &self->inputs[n]->type(), origin);
			input->required_rescls = self->inputs[n]->required_rescls;
		}
	}
	
	for (size_t n = new_node->noutputs; n < self->noutputs; n++) {
		if (self->outputs[n]->gate) {
			jive::gate * gate = self->outputs[n]->gate;

			jive::gate * target_gate = substitution.lookup(gate);
			if (!target_gate) {
				target_gate = jive_graph_create_gate(target->graph, gate->name, gate->type());
				target_gate->required_rescls = gate->required_rescls;
				substitution.insert(gate, target_gate);
			}

			jive_node_gate_output(new_node, target_gate);
		} else {
			jive::output * output = jive_node_add_output(new_node, &self->outputs[n]->type());
			output->required_rescls = self->outputs[n]->required_rescls;
		}
	}
	
	for (size_t n = 0; n < new_node->noutputs; n++)
		substitution.insert(self->outputs[n], new_node->outputs[n]);

	return new_node;
}

jive_node::~jive_node()
{
	graph->on_node_destroy(this);

	JIVE_DEBUG_ASSERT(region);

	JIVE_LIST_REMOVE(region->nodes, this, region_nodes_list);

	while(noutputs)
		delete outputs[noutputs-1];

	while (ninputs)
		delete inputs[ninputs-1];

	JIVE_LIST_REMOVE(graph->bottom, this, graph_bottom_list);
	JIVE_LIST_REMOVE(region->top_nodes, this, region_top_node_list);

	if (this == region->top)
		region->top = nullptr;
	if (this == region->bottom)
		region->bottom = nullptr;

	region = nullptr;

	for (size_t n = 0; n < tracker_slots.size(); n++)
		delete tracker_slots[n];
}

void
jive_node_destroy(jive_node * self)
{
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
			jive_node * node = input->node();
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
	jive_node * node = new jive_node(op.copy());
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
