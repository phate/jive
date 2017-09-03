/*
 * Copyright 2010 2011 2012 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <string.h>

#include <jive/common.h>

#include <jive/util/list.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/graph-private.h>
#include <jive/vsdg/node-normal-form.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/resource.h>
#include <jive/vsdg/simple_node.h>
#include <jive/vsdg/substitution.h>

namespace jive {

/* input */

input::~input() noexcept
{
	origin()->remove_user(this);

	if (port().gate())
		JIVE_LIST_REMOVE(port().gate()->inputs, this, gate_input_list);
}

input::input(
	size_t index,
	jive::output * origin,
	jive::region * region,
	const jive::port & port)
: index_(index)
, port_(port)
, origin_(origin)
, region_(region)
{
	gate_input_list.prev = gate_input_list.next = nullptr;

	if (region != origin->region())
		throw jive::compiler_error("Invalid operand region.");

	if (port.type() != origin->type())
		throw jive::type_error(port.type().debug_string(), origin->type().debug_string());

	if (port.gate())
		JIVE_LIST_PUSH_BACK(port.gate()->inputs, this, gate_input_list);

	origin->add_user(this);
}

std::string
input::debug_string() const
{
	if (port().gate())
		return port().gate()->debug_string();

	return detail::strfmt(index());
}

void
input::divert_origin(jive::output * new_origin)
{
	if (type() != new_origin->type())
		throw jive::type_error(type().debug_string(), new_origin->type().debug_string());

	if (region() != new_origin->region())
		throw jive::compiler_error("Invalid operand region.");

	auto old_origin = origin();
	old_origin->remove_user(this);
	this->origin_ = new_origin;
	new_origin->add_user(this);

	if (node()) node()->recompute_depth(this);
	region()->graph()->mark_denormalized();
	region()->graph()->on_input_change(this, old_origin, new_origin);
}

/* output */

output::~output()
{
	JIVE_DEBUG_ASSERT(nusers() == 0);

	if (port().gate())
		JIVE_LIST_REMOVE(port().gate()->outputs, this, gate_output_list);
}

output::output(
	size_t index,
	jive::region * region,
	const jive::port & port)
: index_(index)
, port_(port)
, region_(region)
{
	gate_output_list.prev = gate_output_list.next = nullptr;

	if (port.gate())
		JIVE_LIST_PUSH_BACK(port.gate()->outputs, this, gate_output_list);
}

std::string
output::debug_string() const
{
	if (port().gate())
		return port().gate()->debug_string();

	return detail::strfmt(index());
}

void
output::remove_user(jive::input * user)
{
	JIVE_DEBUG_ASSERT(users_.find(user) != users_.end());

	users_.erase(user);
	if (node() && !node()->has_users())
		JIVE_LIST_PUSH_BACK(region()->bottom_nodes, node(), region_bottom_list);
}

void
output::add_user(jive::input * user)
{
	JIVE_DEBUG_ASSERT(users_.find(user) == users_.end());

	if (node() && !node()->has_users())
		JIVE_LIST_REMOVE(region()->bottom_nodes, node(), region_bottom_list);
	users_.insert(user);
}

}	//jive namespace

jive::node_normal_form *
jive_node_get_default_normal_form_(
	const std::type_info & operator_class,
	jive::node_normal_form * parent,
	jive::graph * graph)
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

namespace jive {

node::node(std::unique_ptr<jive::operation> op, jive::region * region)
	: depth_(0)
	, graph_(region->graph())
	, region_(region)
	, operation_(std::move(op))
{
	JIVE_LIST_PUSH_BACK(region->top_nodes, this, region_top_node_list);
	JIVE_LIST_PUSH_BACK(region->bottom_nodes, this, region_bottom_list);
	region->nodes.push_back(this);
}

node::~node()
{
	while (ninputs())
		remove_input(ninputs()-1);

	while(noutputs())
		remove_output(noutputs()-1);

	region_->nodes.erase(this);

	JIVE_LIST_REMOVE(region()->bottom_nodes, this, region_bottom_list);
	JIVE_LIST_REMOVE(region_->top_nodes, this, region_top_node_list);

	region_ = nullptr;

	for (size_t n = 0; n < tracker_slots.size(); n++)
		delete tracker_slots[n];
}

void
node::add_input(std::unique_ptr<jive::input> input)
{
	if (ninputs() == 0)
		JIVE_LIST_REMOVE(region()->top_nodes, this, region_top_node_list);

	auto origin = input->origin();
	if (origin->node() && !origin->node()->has_users())
		JIVE_LIST_REMOVE(origin->node()->region()->bottom_nodes, origin->node(), region_bottom_list);

	inputs_.push_back(std::move(input));
	recompute_depth(inputs_.back().get());
}

void
node::remove_input(size_t index)
{
	JIVE_DEBUG_ASSERT(index < ninputs());

	for (size_t n = index; n < ninputs()-1; n++) {
		JIVE_DEBUG_ASSERT(input(n+1)->index() == n);
		inputs_[n] = std::move(inputs_[n+1]);
	}
	inputs_.pop_back();

	if (ninputs() == 0)
		JIVE_LIST_PUSH_BACK(region()->top_nodes, this, region_top_node_list);
}

void
node::remove_output(size_t index)
{
	JIVE_DEBUG_ASSERT(index < noutputs());

	for (size_t n = index; n < noutputs()-1; n++) {
		JIVE_DEBUG_ASSERT(output(n+1)->index() == n);
		outputs_[n] = std::move(outputs_[n+1]);
	}
	outputs_.pop_back();
}

void
node::recompute_depth(jive::input * input)
{
	if (input->node() != this
	|| input->origin()->node() == nullptr
	|| input->origin()->node()->depth() < depth())
		return;

	size_t old_depth = depth_;
	depth_ = input->origin()->node()->depth()+1;
	graph()->on_node_depth_change(this, old_depth);

	for (size_t n = 0; n < noutputs(); n++) {
		for (auto user : *output(n)) {
			if (user->node())
				user->node()->recompute_depth(user);
		}
	}
}

}

static bool
jive_node_cse_test(
	jive::node * node,
	const jive::operation & op,
	const std::vector<jive::output*> & arguments)
{
	return (node->operation() == op && arguments == jive_node_arguments(node));
}

jive::node *
jive_node_cse(
	jive::region * region,
	const jive::operation & op,
	const std::vector<jive::output*> & arguments)
{
	if (!arguments.empty()) {
		for (const auto & user : *arguments[0]) {
			auto node = user->node();
			if (node && jive_node_cse_test(node, op, arguments))
				return node;
		}
	} else {
		jive::node * node;
		JIVE_LIST_ITERATE(region->top_nodes, node, region_top_node_list)
		if (jive_node_cse_test(node, op, arguments))
			return node;
	}

	return nullptr;
}

jive_tracker_nodestate *
jive_node_get_tracker_state_slow(jive::node * self, jive_tracker_slot slot)
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

bool
jive_node_normalize(jive::node * self)
{
	auto nf = self->graph()->node_normal_form(typeid(self->operation()));
	return nf->normalize_node(self);
}
