/*
 * Copyright 2010 2011 2012 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

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

/*
	FIXME: merge with jive_node_valid_edge when we transformed to new representation
*/
static bool
jive_input_is_valid(const jive::input * input)
{
	jive::region * region = input->region();
	jive::region * origin_region = input->origin()->region();

	if (dynamic_cast<const jive::achr::type*>(&input->type()))
		return origin_region->parent() == region;

	if (dynamic_cast<const jive::region_head_op*>(&input->node()->operation()))
		return origin_region == region->parent();

	return origin_region == region;
}

namespace jive {

/* iport */

iport::~iport() noexcept
{
	if (gate())
		JIVE_LIST_REMOVE(gate()->iports, this, gate_iport_list);
}

iport::iport(size_t index, jive::oport * origin, jive::gate * gate) noexcept
	: index_(index)
	, gate_(gate)
	, origin_(origin)
{
	gate_iport_list.prev = gate_iport_list.next = nullptr;
	if (gate)
		JIVE_LIST_PUSH_BACK(gate->iports, this, gate_iport_list);
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
	const jive::base::type * input_type = &this->type();
	const jive::base::type * operand_type = &new_origin->type();

	if (*input_type != *operand_type)
		throw jive::type_error(input_type->debug_string(), operand_type->debug_string());

	if (dynamic_cast<const jive::achr::type*>(input_type)) {
		throw jive::compiler_error("Type mismatch: Cannot divert edges of 'anchor' type");
	}

	/* FIXME: This should be before we assign the new origin */
	if (!jive_input_is_valid(dynamic_cast<jive::input*>(this)))
		throw jive::compiler_error("Invalid input");

	origin()->users.erase(this);
	auto output = dynamic_cast<jive::output*>(origin());
	if (output && !output->node()->has_successors())
		JIVE_LIST_PUSH_BACK(output->node()->graph()->bottom, output->node(), graph_bottom_list);

	this->origin_ = new_origin;

	output = dynamic_cast<jive::output*>(origin());
	if (output && !output->node()->has_successors())
		JIVE_LIST_REMOVE(output->node()->graph()->bottom, output->node(), graph_bottom_list);
	origin()->users.insert(this);

	new_origin->region()->graph()->mark_denormalized();
}

/* inputs */

input::input(
	jive::node * node,
	size_t index,
	jive::oport * origin,
	const jive::base::type & type)
	: iport(index, origin, nullptr)
	, node_(node)
	, rescls_(&jive_root_resource_class)
	, type_(type.copy())
{
	auto output = dynamic_cast<jive::output*>(origin);
	bool has_successors = output->node()->has_successors();

	/* FIXME: check whether origin is valid */
	/* FIXME: This should optimally be in node constructor */

	if (this->type() != origin->type())
		throw jive::type_error(this->type().debug_string(), origin->type().debug_string());

	/*
		FIXME: This is going to be removed once we switched Jive to the new node representation.
	*/
	if (dynamic_cast<const jive::achr::type*>(&type)) {
		JIVE_DEBUG_ASSERT(dynamic_cast<jive::output*>(origin)->node()->region()->anchor() == nullptr);
		dynamic_cast<jive::output*>(origin)->node()->region()->set_anchor(this);
	}

	origin->users.insert(this);
	if (output && !has_successors)
		JIVE_LIST_REMOVE(output->node()->graph()->bottom, output->node(), graph_bottom_list);
}

input::input(
	jive::node * node,
	size_t index,
	jive::oport * origin,
	jive::gate * gate)
	: iport(index, origin, gate)
	, node_(node)
	, rescls_(gate->required_rescls)
	, type_(gate->type().copy())
{
	auto output = dynamic_cast<jive::output*>(origin);
	bool has_successors = output->node()->has_successors();

	/* FIXME: check whether origin is valid */

	if (type() != origin->type())
		throw jive::type_error(type().debug_string(), origin->type().debug_string());

	/*
		FIXME: This is going to be removed once we switched Jive to the new node representation.
	*/
	if (dynamic_cast<const jive::achr::type*>(&type())) {
		JIVE_DEBUG_ASSERT(dynamic_cast<jive::output*>(origin)->node()->region()->anchor() == nullptr);
		dynamic_cast<jive::output*>(origin)->node()->region()->set_anchor(this);
	}

	for (size_t n = 0; n < index; n++) {
		jive::input * other = node->input(n);
		if (!other->gate()) continue;
		jive_gate_interference_add(node->graph(), gate, other->gate());
	}

	origin->users.insert(this);
	if (output && !has_successors)
		JIVE_LIST_REMOVE(output->node()->graph()->bottom, output->node(), graph_bottom_list);
}

input::input(
	jive::node * node,
	size_t index,
	jive::oport * origin,
	const struct jive_resource_class * rescls)
	: iport(index, origin, nullptr)
	, node_(node)
	, rescls_(rescls)
	, type_(jive_resource_class_get_type(rescls)->copy())
{
	auto output = dynamic_cast<jive::output*>(origin);
	bool has_successors = output->node()->has_successors();

	/* FIXME: check whether origin is valid */

	if (type() != origin->type())
		throw jive::type_error(type().debug_string(), origin->type().debug_string());

	/*
		FIXME: This is going to be removed once we switched Jive to the new node representation.
	*/
	if (dynamic_cast<const jive::achr::type*>(&type())) {
		JIVE_DEBUG_ASSERT(dynamic_cast<jive::output*>(origin)->node()->region()->anchor() == nullptr);
		dynamic_cast<jive::output*>(origin)->node()->region()->set_anchor(this);
	}

	origin->users.insert(this);
	if (has_successors)
		JIVE_LIST_REMOVE(output->node()->graph()->bottom, output->node(), graph_bottom_list);
}

input::~input() noexcept
{
	node()->graph()->on_input_destroy(this);

	origin()->users.erase(this);
	auto output = dynamic_cast<jive::output*>(origin());
	if (output && !output->node()->has_successors())
		JIVE_LIST_PUSH_BACK(output->node()->graph()->bottom, output->node(), graph_bottom_list);

	if (gate()) {
		for (size_t n = 0; n < node()->ninputs(); n++) {
			jive::input * other = node()->input(n);
			if (other == this || !other->gate())
				continue;
			jive_gate_interference_remove(node()->graph(), gate(), other->gate());
		}
	}

	for (size_t n = index()+1; n < node()->ninputs(); n++)
		node()->input(n)->set_index(n-1);
}

const jive::base::type &
input::type() const noexcept
{
	return *type_;
}

jive::region *
input::region() const noexcept
{
	return node()->region();
}

jive::node *
input::node() const noexcept
{
	return node_;
}

void
input::divert_origin(jive::oport * new_origin)
{
	jive::oport * old_origin = this->origin();

	iport::divert_origin(new_origin);
	node()->recompute_depth();

	node()->graph()->on_input_change(this, dynamic_cast<jive::output*>(old_origin),
		dynamic_cast<jive::output*>(new_origin));
}

/* oport */

oport::~oport()
{
	if (gate())
		JIVE_LIST_REMOVE(gate()->oports, this, gate_oport_list);
}

oport::oport(size_t index, jive::gate * gate)
	: index_(index)
	, gate_(gate)
{
	gate_oport_list.prev = gate_oport_list.next = nullptr;
	if (gate)
		JIVE_LIST_PUSH_BACK(gate->oports, this, gate_oport_list);
}

std::string
oport::debug_string() const
{
	if (gate())
		return gate()->debug_string();

	return detail::strfmt(index());
}

/* outputs */

output::output(jive::node * node, size_t index, const jive::base::type & type)
	: oport(index, nullptr)
	, node_(node)
	, rescls_(&jive_root_resource_class)
	, type_(type.copy())
{}

output::output(jive::node * node, size_t index, jive::gate * gate)
	: oport(index, gate)
	, node_(node)
	, rescls_(gate->required_rescls)
	, type_(gate->type().copy())
{
	for (size_t n = 0; n < index; n++) {
		jive::output * other = node->output(n);
		if (!other->gate()) continue;
		jive_gate_interference_add(node->graph(), gate, other->gate());
	}
}

output::output(jive::node * node, size_t index, const struct jive_resource_class * rescls)
	: oport(index, nullptr)
	, node_(node)
	, rescls_(rescls)
	, type_(jive_resource_class_get_type(rescls)->copy())
{}

output::~output() noexcept
{
	JIVE_DEBUG_ASSERT(users.empty());

	node_->graph()->on_output_destroy(this);

	if (gate()) {
		for (size_t n = 0; n < node()->noutputs(); n++) {
			jive::output * other = node()->output(n);
			if (other == this || !other->gate())
				continue;
			jive_gate_interference_remove(node()->graph(), gate(), other->gate());
		}
	}

	for (size_t n = index()+1; n < node()->noutputs(); n++)
		node()->output(n)->set_index(n-1);
}

const jive::base::type &
output::type() const noexcept
{
	return *type_;
}

jive::region *
output::region() const noexcept
{
	return node()->region();
}

jive::node *
output::node() const noexcept
{
	return node_;
}

/* gates */

gate::gate(jive_graph * graph, const char name[], const jive::base::type & type)
	: name_(name)
	, graph_(graph)
	, type_(type.copy())
{
	iports.first = iports.last = nullptr;
	oports.first = oports.last = nullptr;
	may_spill = true;
	graph_gate_list.prev = graph_gate_list.next = nullptr;
	required_rescls = &jive_root_resource_class;

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
jive_node_valid_edge(const jive::node * self, const jive::oport * origin)
{
	jive::region * origin_region = origin->region();
	jive::region * target_region = self->region();
	if (dynamic_cast<const jive::achr::type*>(&origin->type()))
		origin_region = origin_region->parent();
	while (target_region) {
		if (target_region == origin_region)
			return true;
		target_region = target_region->parent();
	}
	return false;
}

void
jive_node_get_use_count_input(const jive::node * self, jive_resource_class_count * use_count)
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
		if (input->gate()) rescls = input->gate()->required_rescls;
		else rescls = input->rescls();
		
		use_count->add(rescls);
	}
}

void
jive_node_get_use_count_output(const jive::node * self, jive_resource_class_count * use_count)
{
	use_count->clear();
	
	for (size_t n = 0; n < self->noutputs(); n++) {
		jive::output * output = self->output(n);
		
		const jive_resource_class * rescls;
		if (output->gate()) rescls = output->gate()->required_rescls;
		else rescls = output->rescls();
		
		use_count->add(rescls);
	}
}

namespace jive {

node::node(
	std::unique_ptr<jive::operation> op,
	jive::region * region,
	const std::vector<jive::oport*> & operands)
	: depth_(0)
	, graph_(region->graph())
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
			inputs_.push_back(new jive::input(this, n, operands[n], operation_->argument_type(n)));
			depth_ = std::max(dynamic_cast<jive::output*>(operands[n])->node()->depth()+1, depth_);
		}
	}

	for (size_t n = 0; n < operation_->nresults(); n++) {
		outputs_.push_back(new jive::output(this, n, operation_->result_type(n)));
	}

	for (size_t n = 0; n < this->ninputs(); ++n)
		JIVE_DEBUG_ASSERT(jive_node_valid_edge(this, this->input(n)->origin()));

	region->nodes.push_back(this);
	JIVE_LIST_PUSH_BACK(graph_->bottom, this, graph_bottom_list);

	graph_->on_node_create(this);
}

node::~node()
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

	if (this == region()->top())
		region()->set_top(nullptr);
	if (this == region()->bottom())
		region()->set_bottom(nullptr);

	region_ = nullptr;

	for (size_t n = 0; n < tracker_slots.size(); n++)
		delete tracker_slots[n];
}

bool
node::has_successors() const noexcept
{
	for (auto output : outputs_) {
		if (!output->no_user())
			return true;
	}

	return false;
}

jive::input *
node::add_input(const jive::base::type * type, jive::oport * origin)
{
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
node::add_input(jive::gate * gate, jive::oport * origin)
{
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
node::add_input(const struct jive_resource_class * rescls, jive::oport * origin)
{
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
node::remove_input(size_t index)
{
	JIVE_DEBUG_ASSERT(index < inputs_.size());
	jive::input * input = inputs_[index];

	delete input;
	for (size_t n = index; n < inputs_.size()-1; n++) {
		JIVE_DEBUG_ASSERT(inputs_[n+1]->index() == n);
		inputs_[n] = inputs_[n+1];
	}
	inputs_.pop_back();

	if (inputs_.size() == 0)
		JIVE_LIST_PUSH_BACK(region()->top_nodes, this, region_top_node_list);
}

void
node::remove_output(size_t index)
{
	JIVE_DEBUG_ASSERT(index < outputs_.size());
	jive::output * output = outputs_[index];

	delete output;
	for (size_t n = index; n < outputs_.size()-1; n++) {
		JIVE_DEBUG_ASSERT(outputs_[n+1]->index() == n);
		outputs_[n] = outputs_[n+1];
	}
	outputs_.pop_back();
}

jive::output *
node::add_output(const jive::base::type * type)
{
	jive::output * output = new jive::output(this, outputs_.size(), *type);
	outputs_.push_back(output);

	graph()->on_output_create(output);

	return output;
}

jive::output *
node::add_output(jive::gate * gate)
{
	jive::output * output = new jive::output(this, outputs_.size(), gate);
	outputs_.push_back(output);

	graph()->on_output_create(output);

	return output;
}

jive::output *
node::add_output(const struct jive_resource_class * rescls)
{
	jive::output * output = new jive::output(this, outputs_.size(), rescls);
	outputs_.push_back(output);

	graph()->on_output_create(output);

	return output;
}

jive::node *
node::copy(jive::region * region, const std::vector<jive::oport*> & operands) const
{
	jive::node * node = jive_opnode_create(*operation_, region, operands);
	graph()->mark_denormalized();
	return node;
}

jive::node *
node::copy(jive::region * region, jive::substitution_map & smap) const
{
	std::vector<jive::oport*> operands(noperands());
	for (size_t n = 0; n < noperands(); n++) {
		operands[n] = smap.lookup(dynamic_cast<jive::output*>(input(n)->origin()));
		if (!operands[n]) {
			operands[n] = dynamic_cast<jive::output*>(input(n)->origin());
		}
	}

	jive::node * new_node = copy(region, operands);
	for (size_t n = noperands(); n < ninputs(); n++) {
		jive::oport * origin = smap.lookup(input(n)->origin());
		if (!origin) {
			origin = input(n)->origin();
		}

		if (input(n)->gate()) {
			jive::gate * gate = input(n)->gate();

			jive::gate * target_gate = smap.lookup(gate);
			if (!target_gate) {
				target_gate = region->graph()->create_gate(gate->type(), gate->name());
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
				target_gate = region->graph()->create_gate(gate->type(), gate->name());
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
node::recompute_depth()
{
	size_t new_depth = 0;
	for (size_t n = 0; n < ninputs(); n++)
		new_depth = std::max(dynamic_cast<jive::output*>(input(n)->origin())->node()->depth() + 1,
			new_depth);

	size_t old_depth = depth_;
	if (new_depth == old_depth)
		return;

	depth_ = new_depth;
	graph()->on_node_depth_change(this, old_depth);

	for (size_t n = 0; n < noutputs(); n++) {
		for (auto user : output(n)->users) {
			auto input = dynamic_cast<jive::input*>(user);
			input->node()->recompute_depth();
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
		for (auto user : arguments[0]->users) {
			auto input = dynamic_cast<jive::input*>(user);
			jive::node * node = input->node();
			if (jive_node_cse_test(node, op, arguments)) {
				return node;
			}
		}
	} else {
		jive::node * node;
		JIVE_LIST_ITERATE(region->top_nodes, node, region_top_node_list)
		if (jive_node_cse_test(node, op, arguments))
			return node;
	}

	return nullptr;
}

std::vector<jive::oport*>
jive_node_create_normalized(
	jive::region * region,
	const jive::operation & op,
	const std::vector<jive::oport*> & arguments)
{
	auto nf = region->graph()->node_normal_form(typeid(op));
	return nf->normalized_create(region, op, arguments);
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

jive::node *
jive_node_cse_create(
	const jive::node_normal_form * nf,
	jive::region * region,
	const jive::operation & op,
	const std::vector<jive::oport*> & arguments)
{
	jive::node * node;
	if (nf->get_mutable() && nf->get_cse()) {
		node = jive_node_cse(region, op, arguments);
		if (node) {
			return node;
		}
	}

	return jive_opnode_create(op, region, arguments);
}

bool
jive_node_normalize(jive::node * self)
{
	auto nf = self->graph()->node_normal_form(typeid(self->operation()));
	return nf->normalize_node(self);
}

jive::node *
jive_opnode_create(
	const jive::operation & op,
	jive::region * region,
	const std::vector<jive::oport*> & operands)
{
	jive::node * node = new jive::node(op.copy(), region, operands);

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
		JIVE_DEBUG_ASSERT(!region->top());
		region->set_top(node);
	} else if (dynamic_cast<const jive::region_tail_op *>(&op)) {
		JIVE_DEBUG_ASSERT(!region->bottom());
		region->set_bottom(node);
	}

	for (size_t n = 0; n < node->ninputs(); n++) {
		if (!jive_input_is_valid(node->input(n)))
			throw jive::compiler_error("Invalid input");
	}

	return node;
}
