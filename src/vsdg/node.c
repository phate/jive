/*
 * Copyright 2010 2011 2012 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <string.h>

#include <jive/common.h>

#include <jive/internal/compiler.h>
#include <jive/util/list.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/gate-interference-private.h>
#include <jive/vsdg/graph-private.h>
#include <jive/vsdg/node-normal-form.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/resource.h>
#include <jive/vsdg/simple_node.h>
#include <jive/vsdg/substitution.h>

namespace jive {

/* iport */

iport::~iport() noexcept
{
	origin()->remove_user(this);

	if (gate())
		JIVE_LIST_REMOVE(gate()->iports, this, gate_iport_list);
}

iport::iport(
	size_t index,
	jive::oport * origin,
	const jive::region * region,
	const jive::base::type & type)
	: index_(index)
	, gate_(nullptr)
	, origin_(origin)
	, rescls_(&jive_root_resource_class)
{
	gate_iport_list.prev = gate_iport_list.next = nullptr;

	if (region != origin->region())
		throw jive::compiler_error("Invalid operand region.");

	if (type != origin->type())
		throw jive::type_error(type.debug_string(), origin->type().debug_string());

	origin->add_user(this);
}

iport::iport(
	size_t index,
	jive::oport * origin,
	const jive::region * region,
	jive::gate * gate)
	: index_(index)
	, gate_(gate)
	, origin_(origin)
	, rescls_(gate->rescls())
{
	gate_iport_list.prev = gate_iport_list.next = nullptr;

	if (region != origin->region())
		throw jive::compiler_error("Invalid operand region.");

	if (gate->type() != origin->type())
		throw jive::type_error(gate->type().debug_string(), origin->type().debug_string());

	JIVE_LIST_PUSH_BACK(gate->iports, this, gate_iport_list);

	origin->add_user(this);
}

iport::iport(
	size_t index,
	jive::oport * origin,
	const jive::region * region,
	const struct jive_resource_class * rescls)
	: index_(index)
	, gate_(nullptr)
	, origin_(origin)
	, rescls_(rescls)
{
	gate_iport_list.prev = gate_iport_list.next = nullptr;

	if (region != origin->region())
		throw jive::compiler_error("Invalid operand region.");

	auto type = jive_resource_class_get_type(rescls);
	if (*type != origin->type())
		throw jive::type_error(type->debug_string(), origin->type().debug_string());

	origin->add_user(this);
}

std::string
iport::debug_string() const
{
	if (gate())
		return gate()->debug_string();

	return detail::strfmt(index());
}

void
iport::divert_origin(jive::oport * new_origin)
{
	if (type() != new_origin->type())
		throw jive::type_error(type().debug_string(), new_origin->type().debug_string());

	if (region() != new_origin->region())
		throw jive::compiler_error("Invalid operand region.");

	auto old_origin = origin();
	old_origin->remove_user(this);
	this->origin_ = new_origin;
	new_origin->add_user(this);

	if (node()) node()->recompute_depth();
	region()->graph()->mark_denormalized();
	region()->graph()->on_iport_change(this, old_origin, new_origin);
}

/* oport */

oport::~oport()
{
	JIVE_DEBUG_ASSERT(nusers() == 0);

	if (gate())
		JIVE_LIST_REMOVE(gate()->oports, this, gate_oport_list);
}

oport::oport(size_t index)
	: index_(index)
	, gate_(nullptr)
	, rescls_(&jive_root_resource_class)
{
	gate_oport_list.prev = gate_oport_list.next = nullptr;
}

oport::oport(size_t index, jive::gate * gate)
	: index_(index)
	, gate_(gate)
	, rescls_(gate->rescls())
{
	gate_oport_list.prev = gate_oport_list.next = nullptr;
	JIVE_LIST_PUSH_BACK(gate->oports, this, gate_oport_list);
}

oport::oport(size_t index, const struct jive_resource_class * rescls)
	: index_(index)
	, gate_(nullptr)
	, rescls_(rescls)
{}

std::string
oport::debug_string() const
{
	if (gate())
		return gate()->debug_string();

	return detail::strfmt(index());
}

void
oport::remove_user(jive::iport * user)
{
	JIVE_DEBUG_ASSERT(users_.find(user) != users_.end());

	users_.erase(user);
	if (node() && !node()->has_users())
		JIVE_LIST_PUSH_BACK(region()->bottom_nodes, node(), region_bottom_list);
}

void
oport::add_user(jive::iport * user)
{
	JIVE_DEBUG_ASSERT(users_.find(user) == users_.end());

	if (node() && !node()->has_users())
		JIVE_LIST_REMOVE(region()->bottom_nodes, node(), region_bottom_list);
	users_.insert(user);
}

/* gates */

gate::gate(
	jive::graph * graph,
	const std::string & name,
	const jive::base::type & type)
	: name_(name)
	, graph_(graph)
	, rescls_(&jive_root_resource_class)
	, type_(type.copy())
{
	iports.first = iports.last = nullptr;
	oports.first = oports.last = nullptr;
	may_spill = true;
	graph_gate_list.prev = graph_gate_list.next = nullptr;

	JIVE_LIST_PUSH_BACK(graph->gates, this, graph_gate_list);
}

gate::gate(
	jive::graph * graph,
	const std::string & name,
	const struct jive_resource_class * rescls)
	: name_(name)
	, graph_(graph)
	, rescls_(rescls)
	, type_(jive_resource_class_get_type(rescls)->copy())
{
	iports.first = iports.last = nullptr;
	oports.first = oports.last = nullptr;
	may_spill = true;
	graph_gate_list.prev = graph_gate_list.next = nullptr;

	JIVE_LIST_PUSH_BACK(graph->gates, this, graph_gate_list);
}

gate::~gate() noexcept
{
	JIVE_DEBUG_ASSERT(iports.first == nullptr && iports.last == nullptr);
	JIVE_DEBUG_ASSERT(oports.first == nullptr && oports.last == nullptr);

	JIVE_LIST_REMOVE(graph()->gates, this, graph_gate_list);
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

void
jive_node_get_use_count_input(const jive::node * self, jive_resource_class_count * use_count)
{
	use_count->clear();
	
	for (size_t n = 0; n < self->ninputs(); n++) {
		jive::input * input = dynamic_cast<jive::input*>(self->input(n));
		
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
		if (input->gate()) rescls = input->gate()->rescls();
		else rescls = input->rescls();
		
		use_count->add(rescls);
	}
}

void
jive_node_get_use_count_output(const jive::node * self, jive_resource_class_count * use_count)
{
	use_count->clear();
	
	for (size_t n = 0; n < self->noutputs(); n++) {
		jive::output * output = dynamic_cast<jive::output*>(self->output(n));
		
		const jive_resource_class * rescls;
		if (output->gate()) rescls = output->gate()->rescls();
		else rescls = output->rescls();
		
		use_count->add(rescls);
	}
}

namespace jive {

node::node(std::unique_ptr<jive::operation> op, jive::region * region, size_t depth)
	: depth_(depth)
	, graph_(region->graph())
	, region_(region)
	, operation_(std::move(op))
{
	if (operation().narguments() == 0)
		JIVE_LIST_PUSH_BACK(region->top_nodes, this, region_top_node_list);
	else
		region_top_node_list.prev = region_top_node_list.next = nullptr;

	region->nodes.push_back(this);
	JIVE_LIST_PUSH_BACK(region->bottom_nodes, this, region_bottom_list);
}

node::~node()
{
	region_->nodes.erase(this);

	JIVE_LIST_REMOVE(region()->bottom_nodes, this, region_bottom_list);
	JIVE_LIST_REMOVE(region_->top_nodes, this, region_top_node_list);

	region_ = nullptr;

	for (size_t n = 0; n < tracker_slots.size(); n++)
		delete tracker_slots[n];
}

void
node::recompute_depth()
{
	size_t new_depth = 0;
	for (size_t n = 0; n < ninputs(); n++) {
		if (input(n)->origin()->node())
			new_depth = std::max(input(n)->origin()->node()->depth() + 1, new_depth);
	}

	size_t old_depth = depth_;
	if (new_depth == old_depth)
		return;

	depth_ = new_depth;
	graph()->on_node_depth_change(this, old_depth);

	for (size_t n = 0; n < noutputs(); n++) {
		for (auto user : *output(n)) {
			if (user->node())
				user->node()->recompute_depth();
		}
	}
}

}

static bool
jive_node_cse_test(
	jive::node * node,
	const jive::operation & op,
	const std::vector<jive::oport*> & arguments)
{
	return (node->operation() == op && arguments == jive_node_arguments(node));
}

jive::node *
jive_node_cse(
	jive::region * region,
	const jive::operation & op,
	const std::vector<jive::oport*> & arguments)
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
