/*
 * Copyright 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/graph.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/simple.h>
#include <jive/vsdg/simple_node.h>
#include <jive/vsdg/simple-normal-form.h>
#include <jive/vsdg/substitution.h>

#include <numeric>

namespace jive {

/* inputs */

simple_input::~simple_input() noexcept
{
	node()->graph()->on_input_destroy(this);

	if (port().gate())
		port().gate()->clear_interferences();

	for (size_t n = index()+1; n < node()->ninputs(); n++)
		dynamic_cast<jive::simple_input*>(node()->input(n))->set_index(n-1);
}

simple_input::simple_input(
	jive::simple_node * node,
	size_t index,
	jive::output * origin,
	const jive::port & port)
	: input(index, origin, node->region(), port)
	, node_(node)
{
	if (port.gate()) {
		for (size_t n = 0; n < index; n++) {
			auto other = dynamic_cast<jive::simple_input*>(node->input(n));
			if (!other->port().gate()) continue;
			port.gate()->add_interference(other->port().gate());
		}
	}
}

jive::node *
simple_input::node() const noexcept
{
	return node_;
}

/* outputs */

simple_output::simple_output(
	jive::simple_node * node,
	size_t index,
	const jive::port & port)
: output(index, node->region(), port)
, node_(node)
{
	if (port.gate()) {
		for (size_t n = 0; n < index; n++) {
			auto other = node->output(n);
			if (!other->port().gate()) continue;
			port.gate()->add_interference(other->port().gate());
		}
	}
}

simple_output::~simple_output() noexcept
{
	node_->graph()->on_output_destroy(this);

	if (port().gate())
		port().gate()->clear_interferences();

	for (size_t n = index()+1; n < node()->noutputs(); n++)
		dynamic_cast<jive::simple_output*>(node()->output(n))->set_index(n-1);
}

jive::node *
simple_output::node() const noexcept
{
	return node_;
}

/* simple nodes */

simple_node::~simple_node()
{
	graph()->on_node_destroy(this);

	while(noutputs())
		remove_output(noutputs()-1);
}

simple_node::simple_node(
	const jive::operation & op,
	jive::region * region,
	const std::vector<jive::output*> & operands)
	: node(op.copy(), region, std::accumulate(operands.begin(), operands.end(), 0,
			[](size_t depth, const jive::output * operand) {
				return std::max(depth, operand->node() ? operand->node()->depth()+1 : 0);
			}))
{
	if (operation().narguments() != operands.size())
		throw jive::compiler_error(jive::detail::strfmt("Argument error - expected ",
			operation().narguments(), ", received ", operands.size(), " arguments."));

	for (size_t n = 0; n < operation().narguments(); n++) {
		node::add_input(std::move(std::unique_ptr<jive::input>(
			new simple_input(this, n, operands[n], operation().argument(n)))));
	}

	for (size_t n = 0; n < operation().nresults(); n++)
		outputs_.emplace_back(std::make_unique<jive::simple_output>(this, n, operation().result(n)));

	graph()->on_node_create(this);
}

size_t
simple_node::noutputs() const noexcept
{
	return outputs_.size();
}

jive::simple_output *
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

jive::simple_output *
simple_node::add_output(const jive::port & port)
{
	JIVE_ASSERT(0);
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
simple_node::copy(jive::region * region, const std::vector<jive::output*> & operands) const
{
	jive::node * node = region->add_simple_node(operation(), operands);
	graph()->mark_denormalized();
	return node;
}

jive::node *
simple_node::copy(jive::region * region, jive::substitution_map & smap) const
{
	std::vector<jive::output*> operands(ninputs());
	for (size_t n = 0; n < ninputs(); n++) {
		operands[n] = smap.lookup(input(n)->origin());
		if (!operands[n])
			operands[n] = input(n)->origin();
	}

	auto new_node = copy(region, operands);
	JIVE_DEBUG_ASSERT(new_node->noutputs() == noutputs());

	for (size_t n = 0; n < new_node->noutputs(); n++)
		smap.insert(output(n), new_node->output(n));

	return new_node;
}


std::vector<jive::output*>
create_normalized(
	jive::region * region,
	const jive::simple_op & op,
	const std::vector<jive::output*> & arguments)
{
	auto graph = region->graph();
	auto nf = static_cast<simple_normal_form*>(graph->node_normal_form(typeid(op)));
	return nf->normalized_create(region, op, arguments);
}

}
