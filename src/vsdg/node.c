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
	jive_region * region = input->node()->region();
	jive_region * origin_region = input->origin()->node()->region();

	if (dynamic_cast<const jive::achr::type*>(&input->type()))
		return origin_region->parent == region;

	if (dynamic_cast<const jive::region_head_op*>(&input->node()->operation()))
		return origin_region == region->parent;

	return origin_region == region;
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
	jive::oport * origin,
	const jive::base::type & type)
	: iport(index)
	, gate_(nullptr)
	, origin_(origin)
	, node_(node)
	, ssavar_(nullptr)
	, rescls_(&jive_root_resource_class)
	, type_(type.copy())
{
	if (type != origin->type())
		throw jive::type_error(type.debug_string(), origin->type().debug_string());

	gate_inputs_list.prev = gate_inputs_list.next = nullptr;
	ssavar_input_list.prev = ssavar_input_list.next = nullptr;

	/* FIXME: remove dynamic_cast once we moved users to oport */
	dynamic_cast<jive::output*>(origin)->add_user(this);

	/*
		FIXME: This is going to be removed once we switched Jive to the new node representation.
	*/
	if (dynamic_cast<const jive::achr::type*>(&type)) {
		JIVE_DEBUG_ASSERT(dynamic_cast<jive::output*>(origin)->node()->region()->anchor == nullptr);
		dynamic_cast<jive::output*>(origin)->node()->region()->anchor = this;
	}
}

input::input(
	jive_node * node,
	size_t index,
	jive::oport * origin,
	jive::gate * gate)
	: iport(index)
	, gate_(gate)
	, origin_(origin)
	, node_(node)
	, ssavar_(nullptr)
	, rescls_(gate->required_rescls)
	, type_(gate->type().copy())
{
	if (type() != origin->type())
		throw jive::type_error(type().debug_string(), origin->type().debug_string());

	gate_inputs_list.prev = gate_inputs_list.next = nullptr;
	ssavar_input_list.prev = ssavar_input_list.next = nullptr;

	/* FIXME: remove dynamic_cast once we moved users to oport */
	dynamic_cast<jive::output*>(origin)->add_user(this);

	/*
		FIXME: This is going to be removed once we switched Jive to the new node representation.
	*/
	if (dynamic_cast<const jive::achr::type*>(&type())) {
		JIVE_DEBUG_ASSERT(dynamic_cast<jive::output*>(origin)->node()->region()->anchor == nullptr);
		dynamic_cast<jive::output*>(origin)->node()->region()->anchor = this;
	}

	JIVE_LIST_PUSH_BACK(gate->inputs, this, gate_inputs_list);

	for (size_t n = 0; n < index; n++) {
		jive::input * other = node->input(n);
		if (!other->gate()) continue;
		jive_gate_interference_add(node->graph(), gate, other->gate());
	}
}

input::input(
	struct jive_node * node,
	size_t index,
	jive::oport * origin,
	const struct jive_resource_class * rescls)
	: iport(index)
	, gate_(nullptr)
	, origin_(origin)
	, node_(node)
	, ssavar_(nullptr)
	, rescls_(rescls)
	, type_(jive_resource_class_get_type(rescls)->copy())
{
	if (type() != origin->type())
		throw jive::type_error(type().debug_string(), origin->type().debug_string());

	gate_inputs_list.prev = gate_inputs_list.next = nullptr;
	ssavar_input_list.prev = ssavar_input_list.next = nullptr;

	/* FIXME: remove dynamic_cast once we moved users to oport */
	dynamic_cast<jive::output*>(origin)->add_user(this);

	/*
		FIXME: This is going to be removed once we switched Jive to the new node representation.
	*/
	if (dynamic_cast<const jive::achr::type*>(&type())) {
		JIVE_DEBUG_ASSERT(dynamic_cast<jive::output*>(origin)->node()->region()->anchor == nullptr);
		dynamic_cast<jive::output*>(origin)->node()->region()->anchor = this;
	}
}

input::~input() noexcept
{
	if (ssavar_)
		jive_ssavar_unassign_input(ssavar_, this);

	node()->graph()->on_input_destroy(this);

	/* FIXME: remove dynamic_cast once we moved users to oport */
	dynamic_cast<jive::output*>(origin_)->remove_user(this);
}

const jive::base::type &
input::type() const noexcept
{
	return *type_;
}

std::string
input::debug_string() const
{
	if (gate())
		return gate()->debug_string();

	return iport::debug_string();
}

void
input::divert_origin(jive::output * new_origin) noexcept
{
	JIVE_DEBUG_ASSERT(!this->ssavar());
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

	JIVE_DEBUG_ASSERT(this->node()->graph() == new_origin->node()->graph());
	JIVE_DEBUG_ASSERT(jive_node_valid_edge(this->node(), new_origin));

	jive::output * old_origin = this->origin();

	old_origin->remove_user(this);
	this->origin_ = new_origin;
	new_origin->add_user(this);

	if (!jive_input_is_valid(this))
		throw jive::compiler_error("Invalid input");

	node()->recompute_depth();

	jive_graph_mark_denormalized(new_origin->node()->graph());

	node()->graph()->on_input_change(this, old_origin, new_origin);
}

struct jive_variable *
input::constraint()
{
	if (gate()) {
		jive_variable * variable = gate()->variable;
		if (!variable) {
			variable = jive_variable_create(gate()->graph());
			jive_variable_set_resource_class(variable, gate()->required_rescls);
			jive_variable_assign_gate(variable, gate());
		}
		return variable;
	}

	jive_variable * variable = jive_variable_create(node()->graph());
	jive_variable_set_resource_class(variable, rescls());
	return variable;
}

jive_ssavar *
input::auto_merge_variable()
{
	if (ssavar())
		return ssavar();

	jive_ssavar * ssavar = origin()->ssavar;
	if (ssavar) {
		for (auto user : origin()->users) {
			if (user->ssavar()) {
				ssavar = user->ssavar();
				break;
			}
		}
	}

	if (!ssavar)
		ssavar = jive_ssavar_create(origin(), constraint());

	jive_variable_merge(ssavar->variable, constraint());
	jive_ssavar_assign_input(ssavar, this);
	return ssavar;
}

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
	, ssavar(nullptr)
	, node_(node)
	, gate_(nullptr)
	, rescls_(&jive_root_resource_class)
	, type_(type.copy())
{
	originating_ssavars.first = originating_ssavars.last = nullptr;
	gate_outputs_list.prev = gate_outputs_list.next = nullptr;
}

output::output(jive_node * node, size_t index, jive::gate * gate)
	: oport(index)
	, ssavar(nullptr)
	, node_(node)
	, gate_(gate)
	, rescls_(gate->required_rescls)
	, type_(gate->type().copy())
{
	originating_ssavars.first = originating_ssavars.last = nullptr;
	gate_outputs_list.prev = gate_outputs_list.next = nullptr;

	JIVE_LIST_PUSH_BACK(gate->outputs, this, gate_outputs_list);

	for (size_t n = 0; n < index; n++) {
		jive::output * other = node->output(n);
		if (!other->gate()) continue;
		jive_gate_interference_add(node->graph(), gate, other->gate());
	}
}

output::output(jive_node * node, size_t index, const struct jive_resource_class * rescls)
	: oport(index)
	, ssavar(nullptr)
	, node_(node)
	, gate_(nullptr)
	, rescls_(rescls)
	, type_(jive_resource_class_get_type(rescls)->copy())
{
	originating_ssavars.first = originating_ssavars.last = nullptr;
	gate_outputs_list.prev = gate_outputs_list.next = nullptr;
}

output::~output() noexcept
{
	JIVE_DEBUG_ASSERT(users.empty());

	node_->graph()->on_output_destroy(this);

	if (ssavar)
		jive_ssavar_unassign_output(ssavar, this);

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
	if (gate())
		return gate()->debug_string();

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
		JIVE_LIST_REMOVE(node()->graph()->bottom, node(), graph_bottom_list);

	users.insert(user);
}

void
output::remove_user(jive::input * user) noexcept
{
	users.erase(user);

	if (!node()->has_successors())
		JIVE_LIST_PUSH_BACK(node()->graph()->bottom, node(), graph_bottom_list);
}

/* gates */

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
	jive_region * origin_region = origin->node()->region();
	jive_region * target_region = self->region();
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
jive_node_get_use_count_input(const jive_node * self, jive_resource_class_count * use_count)
{
	use_count->clear();
	
	for (size_t n = 0; n < self->ninputs(); n++) {
		jive::input * input = self->input(n);
		
		/* filter out multiple inputs using the same value
		FIXME: this assumes that all inputs have the same resource
		class requirement! */
		if (!input->gate()) {
			bool duplicate = false;
			size_t k;
			for(k = 0; k<n; k++) {
				if (self->input(k)->origin() == input->origin())
					duplicate = true;
			}
			if (duplicate) continue;
		}
		
		const jive_resource_class * rescls;
		if (input->ssavar()) rescls = input->ssavar()->variable->rescls;
		else if (input->gate()) rescls = input->gate()->required_rescls;
		else rescls = input->rescls();
		
		use_count->add(rescls);
	}
}

void
jive_node_get_use_count_output(const jive_node * self, jive_resource_class_count * use_count)
{
	use_count->clear();
	
	for (size_t n = 0; n < self->noutputs(); n++) {
		jive::output * output = self->output(n);
		
		const jive_resource_class * rescls;
		if (output->ssavar) rescls = output->ssavar->variable->rescls;
		else if (output->gate()) rescls = output->gate()->required_rescls;
		else rescls = output->rescls();
		
		use_count->add(rescls);
	}
}

jive_node::jive_node(
	std::unique_ptr<jive::operation> op,
	jive_region * region,
	const std::vector<jive::output*> & operands)
	: depth_(0)
	, graph_(region->graph)
	, region_(region)
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
			JIVE_DEBUG_ASSERT(!graph_->resources_fully_assigned);
			inputs_.push_back(new jive::input(this, n, operands[n], operation_->argument_type(n)));
			depth_ = std::max(operands[n]->node()->depth()+1, depth_);
		}
	}

	for (size_t n = 0; n < operation_->nresults(); n++) {
		JIVE_DEBUG_ASSERT(!graph_->resources_fully_assigned);
		outputs_.push_back(new jive::output(this, n, operation_->result_type(n)));
	}

	for (size_t n = 0; n < this->ninputs(); ++n)
		JIVE_DEBUG_ASSERT(jive_node_valid_edge(this, this->input(n)->origin()));

	region->nodes.push_back(this);
	JIVE_LIST_PUSH_BACK(graph_->bottom, this, graph_bottom_list);

	graph_->on_node_create(this);
}

jive_node::~jive_node()
{
	graph()->on_node_destroy(this);

	JIVE_DEBUG_ASSERT(region_);

	region_->nodes.erase(this);

	while(outputs_.size())
		remove_output(outputs_.size()-1);

	while (inputs_.size())
		remove_input(inputs_.size()-1);

	JIVE_LIST_REMOVE(graph()->bottom, this, graph_bottom_list);
	JIVE_LIST_REMOVE(region_->top_nodes, this, region_top_node_list);

	if (this == region()->top)
		region()->top = nullptr;
	if (this == region()->bottom)
		region()->bottom = nullptr;

	region_ = nullptr;

	for (size_t n = 0; n < tracker_slots.size(); n++)
		delete tracker_slots[n];
}

jive::input *
jive_node::add_input(const jive::base::type * type, jive::output * origin)
{
	JIVE_DEBUG_ASSERT(!graph()->resources_fully_assigned);
	jive::input * input = new jive::input(this, inputs_.size(), origin, *type);

	if (inputs_.size() == 0)
		JIVE_LIST_REMOVE(region()->top_nodes, this, region_top_node_list);

	inputs_.push_back(input);

	if (!jive_input_is_valid(input))
		throw jive::compiler_error("Invalid input");

	JIVE_DEBUG_ASSERT(jive_node_valid_edge(this, input->origin()));
	recompute_depth();
	graph()->on_input_create(input);

	return input;
}

jive::input *
jive_node::add_input(jive::gate * gate, jive::output * origin)
{
	JIVE_DEBUG_ASSERT(!graph()->resources_fully_assigned);
	jive::input * input = new jive::input(this, inputs_.size(), origin, gate);

	if (inputs_.size() == 0)
		JIVE_LIST_REMOVE(region()->top_nodes, this, region_top_node_list);

	inputs_.push_back(input);

	if (!jive_input_is_valid(input))
		throw jive::compiler_error("Invalid input");

	JIVE_DEBUG_ASSERT(jive_node_valid_edge(this, input->origin()));
	recompute_depth();
	graph()->on_input_create(input);

	return input;
}

jive::input *
jive_node::add_input(const struct jive_resource_class * rescls, jive::output * origin)
{
	JIVE_DEBUG_ASSERT(!graph()->resources_fully_assigned);
	jive::input * input = new jive::input(this, inputs_.size(), origin, rescls);

	if (inputs_.size() == 0)
		JIVE_LIST_REMOVE(region()->top_nodes, this, region_top_node_list);

	inputs_.push_back(input);

	if (!jive_input_is_valid(input))
		throw jive::compiler_error("Invalid input");

	JIVE_DEBUG_ASSERT(jive_node_valid_edge(this, input->origin()));
	recompute_depth();
	graph()->on_input_create(input);

	return input;
}

void
jive_node::remove_input(size_t index)
{
	JIVE_DEBUG_ASSERT(index < inputs_.size());
	jive::input * input = inputs_[index];

	if (input->gate()) {
		JIVE_LIST_REMOVE(input->gate()->inputs, input, gate_inputs_list);

		for (size_t n = 0; n < inputs_.size(); n++) {
			jive::input * other = inputs_[n];
			if (other == input || !other->gate())
				continue;
			jive_gate_interference_remove(graph(), input->gate(), other->gate());
		}
	}

	delete input;
	for (size_t n = index; n < inputs_.size()-1; n++) {
		inputs_[n] = inputs_[n+1];
		inputs_[n]->set_index(n);
	}
	inputs_.pop_back();

	if (inputs_.size() == 0)
		JIVE_LIST_PUSH_BACK(region()->top_nodes, this, region_top_node_list);
}

void
jive_node::remove_output(size_t index)
{
	JIVE_DEBUG_ASSERT(index < outputs_.size());
	jive::output * output = outputs_[index];

	if (output->gate()) {
		JIVE_LIST_REMOVE(output->gate()->outputs, output, gate_outputs_list);

		for (size_t n = 0; n < noutputs(); n++) {
			jive::output * other = outputs_[n];
			if (other == output || !other->gate())
				continue;
			jive_gate_interference_remove(graph(), output->gate(), other->gate());
		}
	}

	delete output;
	for (size_t n = index; n < outputs_.size()-1; n++) {
		outputs_[n] = outputs_[n+1];
		outputs_[n]->set_index(n);
	}
	outputs_.pop_back();
}

jive::output *
jive_node::add_output(const jive::base::type * type)
{
	JIVE_DEBUG_ASSERT(!graph()->resources_fully_assigned);
	jive::output * output = new jive::output(this, outputs_.size(), *type);
	outputs_.push_back(output);

	graph()->on_output_create(output);

	return output;
}

jive::output *
jive_node::add_output(jive::gate * gate)
{
	JIVE_DEBUG_ASSERT(!graph()->resources_fully_assigned);
	jive::output * output = new jive::output(this, outputs_.size(), gate);
	outputs_.push_back(output);

	graph()->on_output_create(output);

	return output;
}

jive::output *
jive_node::add_output(const struct jive_resource_class * rescls)
{
	JIVE_DEBUG_ASSERT(!graph()->resources_fully_assigned);
	jive::output * output = new jive::output(this, outputs_.size(), rescls);
	outputs_.push_back(output);

	graph()->on_output_create(output);

	return output;
}

jive_node *
jive_node::copy(jive_region * region, const std::vector<jive::output*> & operands) const
{
	jive_graph_mark_denormalized(graph());
	return operation_->create_node(region, noperands(), &operands[0]);
}

jive_node *
jive_node::copy(jive_region * region, jive::substitution_map & smap) const
{
	std::vector<jive::output*> operands(noperands());
	for (size_t n = 0; n < noperands(); n++) {
		operands[n] = smap.lookup(input(n)->origin());
		if (!operands[n]) {
			operands[n] = input(n)->origin();
		}
	}

	jive_node * new_node = copy(region, operands);
	for (size_t n = noperands(); n < ninputs(); n++) {
		jive::output * origin = smap.lookup(input(n)->origin());
		if (!origin) {
			origin =  input(n)->origin();
		}

		if (input(n)->gate()) {
			jive::gate * gate = input(n)->gate();

			jive::gate * target_gate = smap.lookup(gate);
			if (!target_gate) {
				target_gate = jive_graph_create_gate(region->graph, gate->name(), gate->type());
				target_gate->required_rescls = gate->required_rescls;
				smap.insert(gate, target_gate);
			}
		} else {
			new_node->add_input(this->input(n)->rescls(), origin);
		}
	}

	for (size_t n = new_node->noutputs(); n < noutputs(); n++) {
		if (output(n)->gate()) {
			jive::gate * gate = output(n)->gate();

			jive::gate * target_gate = smap.lookup(gate);
			if (!target_gate) {
				target_gate = jive_graph_create_gate(region->graph, gate->name(), gate->type());
				target_gate->required_rescls = gate->required_rescls;
				smap.insert(gate, target_gate);
			}

			new_node->add_output(target_gate);
		} else {
			new_node->add_output(this->output(n)->rescls());
		}
	}

	for (size_t n = 0; n < new_node->noutputs(); n++)
		smap.insert(output(n), new_node->output(n));

	return new_node;
}

void
jive_node::recompute_depth()
{
	size_t new_depth = 0;
	for (size_t n = 0; n < ninputs(); n++)
		new_depth = std::max(producer(n)->depth() + 1, new_depth);

	size_t old_depth = depth_;
	if (new_depth == old_depth)
		return;

	depth_ = new_depth;
	graph()->on_node_depth_change(this, old_depth);

	for (size_t n = 0; n < noutputs(); n++) {
		for (auto user : output(n)->users)
			user->node()->recompute_depth();
	}
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
	const jive::node_normal_form * nf = jive_graph_get_nodeclass_form(self->graph(),
		typeid(self->operation()));
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
		node->input(n)->set_rescls(op.argument_cls(n));
	}
	for (size_t n = 0; n < op.nresults(); ++n) {
		node->output(n)->set_rescls(op.result_cls(n));
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

	for (size_t n = 0; n < node->ninputs(); n++) {
		if (!jive_input_is_valid(node->input(n)))
			throw jive::compiler_error("Invalid input");
	}

	return node;
}
