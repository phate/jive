/*
 * Copyright 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/gate-interference-private.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/operators/simple-normal-form.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/simple_node.h>
#include <jive/vsdg/substitution.h>

#include <numeric>

namespace jive {

/* inputs */

input::~input() noexcept
{
	node()->graph()->on_iport_destroy(this);

	if (gate()) {
		for (size_t n = 0; n < node()->ninputs(); n++) {
			jive::input * other = dynamic_cast<jive::input*>(node()->input(n));
			if (other == this || !other->gate())
				continue;
			jive_gate_interference_remove(node()->graph(), gate(), other->gate());
		}
	}

	for (size_t n = index()+1; n < node()->ninputs(); n++)
		dynamic_cast<jive::input*>(node()->input(n))->set_index(n-1);
}

input::input(
	jive::node * node,
	size_t index,
	jive::oport * origin,
	const jive::base::type & type)
	: iport(node->region(), type, index, origin)
	, node_(node)
	, type_(type.copy())
{}

input::input(
	jive::node * node,
	size_t index,
	jive::oport * origin,
	jive::gate * gate)
	: iport(node->region(), gate->type(), index, origin, gate)
	, node_(node)
	, type_(gate->type().copy())
{
	for (size_t n = 0; n < index; n++) {
		jive::input * other = dynamic_cast<jive::input*>(node->input(n));
		if (!other->gate()) continue;
		jive_gate_interference_add(node->graph(), gate, other->gate());
	}
}

input::input(
	jive::node * node,
	size_t index,
	jive::oport * origin,
	const jive::base::type & type,
	const struct jive_resource_class * rescls)
	: iport(node->region(), type, index, origin, rescls)
	, node_(node)
	, type_(type.copy())
{}

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

/* outputs */

output::output(jive::node * node, size_t index, const jive::base::type & type)
	: oport(index)
	, node_(node)
	, type_(type.copy())
{}

output::output(jive::node * node, size_t index, jive::gate * gate)
	: oport(index, gate)
	, node_(node)
	, type_(gate->type().copy())
{
	for (size_t n = 0; n < index; n++) {
		jive::output * other = dynamic_cast<jive::output*>(node->output(n));
		if (!other->gate()) continue;
		jive_gate_interference_add(node->graph(), gate, other->gate());
	}
}

output::output(
	jive::node * node,
	size_t index,
	const jive::base::type & type,
	const struct jive_resource_class * rescls)
	: oport(index, rescls)
	, node_(node)
	, type_(type.copy())
{}

output::~output() noexcept
{
	node_->graph()->on_oport_destroy(this);

	if (gate()) {
		for (size_t n = 0; n < node()->noutputs(); n++) {
			jive::output * other = dynamic_cast<jive::output*>(node()->output(n));
			if (other == this || !other->gate())
				continue;
			jive_gate_interference_remove(node()->graph(), gate(), other->gate());
		}
	}

	for (size_t n = index()+1; n < node()->noutputs(); n++)
		dynamic_cast<jive::output*>(node()->output(n))->set_index(n-1);
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

/* simple nodes */

simple_node::~simple_node()
{
	graph()->on_node_destroy(this);

	while (ninputs())
		remove_input(ninputs()-1);

	while(noutputs())
		remove_output(noutputs()-1);
}

simple_node::simple_node(
	const jive::operation & op,
	jive::region * region,
	const std::vector<jive::oport*> & operands)
	: node(op.copy(), region, std::accumulate(operands.begin(), operands.end(), 0,
			[](size_t depth, const jive::oport * operand) {
				return std::max(depth, operand->node() ? operand->node()->depth()+1 : 0);
			}))
{
	if (operation().narguments() != operands.size())
		throw jive::compiler_error(jive::detail::strfmt("Argument error - expected ",
			operation().narguments(), ", received ", operands.size(), " arguments."));

	for (size_t n = 0; n < operation().narguments(); n++) {
		inputs_.emplace_back(std::unique_ptr<jive::input>(new jive::input(
			this, n, operands[n], operation().argument_type(n), operation().argument_cls(n))));
	}

	for (size_t n = 0; n < operation().nresults(); n++) {
		outputs_.emplace_back(std::unique_ptr<jive::output>(new jive::output(this, n,
			operation().result_type(n), operation().result_cls(n))));
	}

	JIVE_DEBUG_ASSERT(operation().narguments() == inputs_.size());

	graph()->on_node_create(this);
}

size_t
simple_node::ninputs() const noexcept
{
	return inputs_.size();
}

jive::input *
simple_node::input(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < ninputs());
	return inputs_[index].get();
}

size_t
simple_node::noutputs() const noexcept
{
	return outputs_.size();
}

jive::output *
simple_node::output(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < noutputs());
	return outputs_[index].get();
}

bool
simple_node::has_users() const noexcept
{
	for (const auto & output : outputs_) {
		if (output->nusers() != 0)
			return true;
	}

	return false;
}

bool
simple_node::has_successors() const noexcept
{
	for (const auto & output : outputs_) {
		for (const auto & user : *output) {
			if (user->node())
				return true;
		}
	}

	return false;
}

jive::input *
simple_node::add_input(const jive::base::type * type, jive::oport * origin)
{
	if (ninputs() == 0)
		JIVE_LIST_REMOVE(region()->top_nodes, this, region_top_node_list);

	inputs_.emplace_back(std::unique_ptr<jive::input>(
		new jive::input(this, ninputs(), origin, *type)));
	auto input = this->input(ninputs()-1);

	recompute_depth();
	graph()->on_iport_create(input);

	return input;
}

jive::input *
simple_node::add_input(jive::gate * gate, jive::oport * origin)
{
	if (ninputs() == 0)
		JIVE_LIST_REMOVE(region()->top_nodes, this, region_top_node_list);

	inputs_.emplace_back(std::unique_ptr<jive::input>(
		new jive::input(this, ninputs(), origin, gate)));
	auto input = this->input(ninputs()-1);

	recompute_depth();
	graph()->on_iport_create(input);

	return input;
}

jive::input *
simple_node::add_input(const struct jive_resource_class * rescls, jive::oport * origin)
{
	if (ninputs() == 0)
		JIVE_LIST_REMOVE(region()->top_nodes, this, region_top_node_list);

	inputs_.emplace_back(std::unique_ptr<jive::input>(
		new jive::input(this, ninputs(), origin, *jive_resource_class_get_type(rescls), rescls)));
	auto input = this->input(ninputs()-1);

	recompute_depth();
	graph()->on_iport_create(input);

	return input;
}

void
simple_node::remove_input(size_t index)
{
	JIVE_DEBUG_ASSERT(index < inputs_.size());

	for (size_t n = index; n < ninputs()-1; n++) {
		JIVE_DEBUG_ASSERT(input(n+1)->index() == n);
		inputs_[n] = std::move(inputs_[n+1]);
	}
	inputs_.pop_back();

	if (ninputs() == 0)
		JIVE_LIST_PUSH_BACK(region()->top_nodes, this, region_top_node_list);
}

jive::output *
simple_node::add_output(const jive::base::type * type)
{
	outputs_.emplace_back(std::unique_ptr<jive::output>(
		new jive::output(this, noutputs(), *type)));
	auto output = outputs_[noutputs()-1].get();

	graph()->on_oport_create(output);

	return output;
}

jive::output *
simple_node::add_output(jive::gate * gate)
{
	outputs_.emplace_back(std::unique_ptr<jive::output>(
		new jive::output(this, noutputs(), gate)));
	auto output = this->output(noutputs()-1);

	graph()->on_oport_create(output);

	return output;
}

jive::output *
simple_node::add_output(const struct jive_resource_class * rescls)
{
	outputs_.emplace_back(std::unique_ptr<jive::output>(
		new jive::output(this, noutputs(), *jive_resource_class_get_type(rescls), rescls)));
	auto output = this->output(noutputs()-1);

	graph()->on_oport_create(output);

	return output;
}

void
simple_node::remove_output(size_t index)
{
	JIVE_DEBUG_ASSERT(index < outputs_.size());

	for (size_t n = index; n < noutputs()-1; n++) {
		JIVE_DEBUG_ASSERT(output(n+1)->index() == n);
		outputs_[n] = std::move(outputs_[n+1]);
	}
	outputs_.pop_back();
}

jive::node *
simple_node::copy(jive::region * region, const std::vector<jive::oport*> & operands) const
{
	jive::node * node = region->add_simple_node(operation(), operands);
	graph()->mark_denormalized();
	return node;
}

jive::node *
simple_node::copy(jive::region * region, jive::substitution_map & smap) const
{
	std::vector<jive::oport*> operands(noperands());
	for (size_t n = 0; n < noperands(); n++) {
		operands[n] = smap.lookup(input(n)->origin());
		if (!operands[n])
			operands[n] = input(n)->origin();
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
				target_gate = region->graph()->create_gate(gate);
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
				target_gate = region->graph()->create_gate(gate);
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


std::vector<jive::oport*>
create_normalized(
	jive::region * region,
	const jive::operation & op,
	const std::vector<jive::oport*> & arguments)
{
	auto graph = region->graph();
	/* FIXME: tighten jive::operation to jive::simple_op and replace dynamic with static cast */
	auto nf = dynamic_cast<simple_normal_form*>(graph->node_normal_form(typeid(op)));
	return nf->normalized_create(region, op, arguments);
}

}
