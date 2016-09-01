/*
 * Copyright 2010 2011 2012 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
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

/*
	FIXME: merge with jive_node_valid_edge when we transformed to new representation
*/
static bool
jive_input_is_valid(const jive::input * input)
{
	jive_region * region = input->node()->region;
	jive_region * origin_region = input->origin()->node()->region;

	if (dynamic_cast<const jive::achr::type*>(&input->type()))
		return origin_region->parent == region;

	if (dynamic_cast<const jive::region_head_op*>(&input->node()->operation()))
		return origin_region == region->parent;

	return origin_region == region;
}

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
		for (auto user : self->outputs[n]->users)
			jive_node_invalidate_depth_from_root(user->node());
	}
}

namespace jive {

/* iport */

iport::~iport() noexcept
{}

iport::iport(size_t index)
	: index_(index)
{}

std::string
iport::debug_string() const
{
	return detail::strfmt(index());
}

/* inputs */

input::input(
	struct jive_node * node,
	size_t index,
	jive::output * origin,
	const jive::base::type & type)
	: iport(index)
	, gate(nullptr)
	, ssavar(nullptr)
	, required_rescls(&jive_root_resource_class)
	, origin_(origin)
	, node_(node)
	, type_(type.copy())
{
	if (type != origin->type())
		throw jive::type_error(type.debug_string(), origin->type().debug_string());

	gate_inputs_list.prev = gate_inputs_list.next = nullptr;
	ssavar_input_list.prev = ssavar_input_list.next = nullptr;

	origin->add_user(this);

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

	origin_->remove_user(this);

	node()->ninputs--;
	for (size_t n = index(); n < node()->ninputs; n++) {
		node()->inputs[n] = node()->inputs[n+1];
		node()->inputs[n]->set_index(n);
	}
	if (node()->ninputs == 0)
		JIVE_LIST_PUSH_BACK(node()->region->top_nodes, node_, region_top_node_list);
}

const jive::base::type &
input::type() const noexcept
{
	return *type_;
}

std::string
input::debug_string() const
{
	if (gate)
		return gate->debug_string();

	return iport::debug_string();
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

	o1->remove_user(this);
	o2->remove_user(other);

	this->origin_ = o2;
	other->origin_ = o1;

	o2->add_user(this);
	o1->add_user(other);

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
	JIVE_DEBUG_ASSERT(jive_node_valid_edge(this->node(), new_origin));

	jive::output * old_origin = this->origin();

	old_origin->remove_user(this);
	this->origin_ = new_origin;
	new_origin->add_user(this);

	if (!jive_input_is_valid(this))
		throw jive::compiler_error("Invalid input");

	jive_node_invalidate_depth_from_root(this->node());

	jive_graph_mark_denormalized(new_origin->node()->graph);

	node()->graph->on_input_change(this, old_origin, new_origin);
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
		for (auto user : self->origin()->users) {
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


namespace jive {

/* oport */

oport::~oport()
{}

oport::oport(size_t index)
	: index_(index)
{}

std::string
oport::debug_string() const
{
	return detail::strfmt(index());
}

/* outputs */

output::output(jive_node * node, size_t index, const jive::base::type & type)
	: oport(index)
	, gate(nullptr)
	, ssavar(nullptr)
	, required_rescls(&jive_root_resource_class)
	, node_(node)
	, type_(type.copy())
{
	originating_ssavars.first = originating_ssavars.last = nullptr;
	gate_outputs_list.prev = gate_outputs_list.next = nullptr;
}

output::~output() noexcept
{
	JIVE_DEBUG_ASSERT(users.empty());

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
	for (size_t n = index(); n < node_->noutputs; n++) {
		node_->outputs[n] = node_->outputs[n+1];
		node_->outputs[n]->set_index(n);
	}

	JIVE_DEBUG_ASSERT(originating_ssavars.first == 0);
}

const jive::base::type &
output::type() const noexcept
{
	return *type_;
}

std::string
output::debug_string() const
{
	if (gate)
		return gate->debug_string();

	return oport::debug_string();
}

void
output::replace(jive::output * other) noexcept
{
	while (users.size())
		(*users.begin())->divert_origin(other);
}

void
output::add_user(jive::input * user) noexcept
{
	if (!node()->has_successors())
		JIVE_LIST_REMOVE(node()->graph->bottom, node(), graph_bottom_list);

	users.insert(user);
}

void
output::remove_user(jive::input * user) noexcept
{
	users.erase(user);

	if (!node()->has_successors())
		JIVE_LIST_PUSH_BACK(node()->graph->bottom, node(), graph_bottom_list);
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
		for (auto user : self->users) {
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
		for (auto user : self->users) {
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

	for (auto user : self->users) {
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

/* gates */

namespace jive {

gate::gate(jive_graph * graph, const char name[], const jive::base::type & type)
	: name_(name)
	, graph_(graph)
	, type_(type.copy())
{
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

void
gate::split()
{
	/* split off this gate from others assigned to the same variable */
	jive_variable * new_variable = jive_variable_create(variable->graph);
	jive_variable_set_resource_class(new_variable, jive_variable_get_resource_class(variable));
	jive_variable_set_resource_name(new_variable, jive_variable_get_resource_name(variable));

	jive_variable_unassign_gate(variable, this);
	jive_variable_assign_gate(new_variable, this);
}

}	//jive namespace

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

void
jive_node_move(jive_node * self, jive_region * new_region)
{
	if (self->region == new_region)
		return;

	/* verify that the node is moved along the region path to the root */
	jive_region * child = new_region;
	jive_region * parent = self->region;
	if (self->region->depth() > new_region->depth()) {
		child = self->region;
		parent = new_region;
	}
	if (!parent->contains(child))
		throw std::logic_error("Node can only be moved along the region path to the root.");

	/* update notion of top nodes of old region */
	if (self->ninputs == 0) {
		JIVE_LIST_REMOVE(self->region->top_nodes, self, region_top_node_list);
	}
		
	/* move the node to the new region */
	JIVE_LIST_REMOVE(self->region->nodes, self, region_nodes_list);
	self->region = new_region;
	JIVE_LIST_PUSH_BACK(self->region->nodes, self, region_nodes_list);

	/* update notion of top nodes of new region */
	for (size_t n = 0; n < self->ninputs; n++) {
		/* if it is an anchor node, we also need to pull/push in/out the corresponding regions */
		if (dynamic_cast<const jive::achr::type*>(&self->inputs[n]->type())) {
			jive_region * subregion = self->producer(n)->region;
			subregion->reparent(new_region);
		}
	}
	if (self->ninputs == 0) {
		JIVE_LIST_PUSH_BACK(self->region->top_nodes, self, region_top_node_list);
	}
}

struct jive_node *
jive_node_copy(const jive_node * self, struct jive_region * region, jive::output * operands[])
{
	jive_graph_mark_denormalized(region->graph);
	return self->operation().create_node(region, self->noperands(), operands);
}

jive_node *
jive_node_copy_substitute(
	const jive_node * self,
	jive_region * target,
	jive::substitution_map & substitution)
{
	std::vector<jive::output*> operands(self->noperands());
	for (size_t n = 0; n < self->noperands(); n++) {
		operands[n] = substitution.lookup(self->inputs[n]->origin());
		if (!operands[n]) {
			operands[n] = self->inputs[n]->origin();
		}
	}
	
	jive_node * new_node = jive_node_copy(self, target, &operands[0]);
	for (size_t n = self->noperands(); n < self->ninputs; n++) {
		jive::output * origin = substitution.lookup(self->inputs[n]->origin());
		if (!origin) {
			origin =  self->inputs[n]->origin();
		}

		if (self->inputs[n]->gate) {
			jive::gate * gate = self->inputs[n]->gate;

			jive::gate * target_gate = substitution.lookup(gate);
			if (!target_gate) {
				target_gate = jive_graph_create_gate(target->graph, gate->name(), gate->type());
				target_gate->required_rescls = gate->required_rescls;
				substitution.insert(gate, target_gate);
			}
		} else {
			jive::input * input = new_node->add_input(&self->inputs[n]->type(), origin);
			input->required_rescls = self->inputs[n]->required_rescls;
		}
	}
	
	for (size_t n = new_node->noutputs; n < self->noutputs; n++) {
		if (self->outputs[n]->gate) {
			jive::gate * gate = self->outputs[n]->gate;

			jive::gate * target_gate = substitution.lookup(gate);
			if (!target_gate) {
				target_gate = jive_graph_create_gate(target->graph, gate->name(), gate->type());
				target_gate->required_rescls = gate->required_rescls;
				substitution.insert(gate, target_gate);
			}

			new_node->add_output(target_gate);
		} else {
			jive::output * output = new_node->add_output(&self->outputs[n]->type());
			output->required_rescls = self->outputs[n]->required_rescls;
		}
	}
	
	for (size_t n = 0; n < new_node->noutputs; n++)
		substitution.insert(self->outputs[n], new_node->outputs[n]);

	return new_node;
}

jive_node::jive_node(
	std::unique_ptr<jive::operation> op,
	jive_region * region,
	const std::vector<jive::output*> & operands)
	: graph(region->graph)
	, region(region)
	, depth_from_root(0)
	, ninputs(0)
	, noutputs(0)
	, operation_(std::move(op))
{
	if (operation_->narguments() != operands.size())
		throw jive::compiler_error(jive::detail::strfmt("Argument error - expected ",
			operation_->narguments(), ", received ", operands.size(), " arguments."));

	if (operation_->narguments() == 0) {
		JIVE_LIST_PUSH_BACK(region->top_nodes, this, region_top_node_list);
	} else {
		region_top_node_list.prev = region_top_node_list.next = nullptr;

		for (size_t n = 0; n < operation_->narguments(); n++) {
			JIVE_DEBUG_ASSERT(!this->graph->resources_fully_assigned);
			jive::input * input = new jive::input(this, n, operands[n], operation_->argument_type(n));
			ninputs++;
			inputs.push_back(input);
			depth_from_root = std::max(operands[n]->node()->depth_from_root+1, depth_from_root);
		}
	}

	for (size_t n = 0; n < operation_->nresults(); n++) {
		JIVE_DEBUG_ASSERT(!graph->resources_fully_assigned);
		jive::output * output = new jive::output(this, noutputs, operation_->result_type(n));
		noutputs++;
		outputs.push_back(output);
	}

	for (size_t n = 0; n < this->ninputs; ++n)
		JIVE_DEBUG_ASSERT(jive_node_valid_edge(this, this->inputs[n]->origin()));

	JIVE_LIST_PUSH_BACK(region->nodes, this, region_nodes_list);
	JIVE_LIST_PUSH_BACK(graph->bottom, this, graph_bottom_list);

	graph->on_node_create(this);
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

jive::input *
jive_node::add_input(const jive::base::type * type, jive::output * origin)
{
	JIVE_DEBUG_ASSERT(!graph->resources_fully_assigned);
	jive::input * input = new jive::input(this, ninputs, origin, *type);

	if (ninputs == 0)
		JIVE_LIST_REMOVE(region->top_nodes, this, region_top_node_list);

	ninputs++;
	inputs.push_back(input);

	if (!jive_input_is_valid(input))
		throw jive::compiler_error("Invalid input");

	JIVE_DEBUG_ASSERT(jive_node_valid_edge(this, input->origin()));
	jive_node_invalidate_depth_from_root(this);
	graph->on_input_create(input);

	return input;
}

jive::input *
jive_node::add_input(jive::gate * gate, jive::output * origin)
{
	jive::input * input = add_input(&gate->type(), origin);
	input->required_rescls = gate->required_rescls;
	input->gate = gate;
	JIVE_LIST_PUSH_BACK(gate->inputs, input, gate_inputs_list);

	for (size_t n = 0; n < input->index(); n++) {
		jive::input * other = inputs[n];
		if (!other->gate) continue;
		jive_gate_interference_add(graph, gate, other->gate);
	}

	return input;
}

jive::output *
jive_node::add_output(const jive::base::type * type)
{
	JIVE_DEBUG_ASSERT(!graph->resources_fully_assigned);
	jive::output * output = new jive::output(this, noutputs, *type);
	noutputs++;
	outputs.push_back(output);

	graph->on_output_create(output);

	return output;
}

jive::output *
jive_node::add_output(jive::gate * gate)
{
	jive::output * output = add_output(&gate->type());
	output->required_rescls = gate->required_rescls;
	output->gate = gate;
	JIVE_LIST_PUSH_BACK(gate->outputs, output, gate_outputs_list);

	for (size_t n = 0; n < output->index(); n++) {
		jive::output * other = outputs[n];
		if (!other->gate) continue;
		jive_gate_interference_add(graph, gate, other->gate);
	}

	return output;
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
		for (auto user : arguments[0]->users) {
			jive_node * node = user->node();
			if (jive_node_cse_test(node, op, arguments)) {
				return node;
			}
		}
	} else {
		jive_node * node;
		JIVE_LIST_ITERATE(region->top_nodes, node, region_top_node_list)
		if (jive_node_cse_test(node, op, arguments))
			return node;
	}

	return nullptr;
}

std::vector<jive::output *>
jive_node_create_normalized(
	jive_region * region,
	const jive::operation & op,
	const std::vector<jive::output *> & arguments)
{
	jive::node_normal_form * nf = jive_graph_get_nodeclass_form(region->graph, typeid(op));
	return nf->normalized_create(region, op, arguments);
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
	std::vector<jive::output*> arguments;
	for (size_t n = 0; n < op.narguments(); ++n) {
		JIVE_DEBUG_ASSERT(args_begin != args_end);
		arguments.push_back(*args_begin);
		++args_begin;
	}
	JIVE_DEBUG_ASSERT(args_begin == args_end);
	
	jive_node * node = new jive_node(op.copy(), region, arguments);

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

	for (size_t n = 0; n < node->ninputs; n++) {
		if (!jive_input_is_valid(node->inputs[n]))
			throw jive::compiler_error("Invalid input");
	}

	return node;
}
