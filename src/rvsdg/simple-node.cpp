/*
 * Copyright 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/rvsdg/graph.hpp>
#include <jive/rvsdg/notifiers.hpp>
#include <jive/rvsdg/region.hpp>
#include <jive/rvsdg/simple-node.hpp>
#include <jive/rvsdg/simple-normal-form.hpp>
#include <jive/rvsdg/substitution.hpp>

#include <numeric>

namespace jive {

/* inputs */

simple_input::~simple_input() noexcept
{
	on_input_destroy(this);
}

simple_input::simple_input(
	jive::simple_node * node,
	jive::output * origin,
	const jive::port & port)
: input(origin, node->region(), port)
, node_(node)
{}

jive::simple_node *
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
{}

simple_output::~simple_output() noexcept
{
	on_output_destroy(this);
}

jive::simple_node *
simple_output::node() const noexcept
{
	return node_;
}

/* simple nodes */

simple_node::~simple_node()
{
	on_node_destroy(this);
}

simple_node::simple_node(
	jive::region * region,
	const jive::simple_op & op,
	const std::vector<jive::output*> & operands)
	: node(op.copy(), region)
{
	if (operation().narguments() != operands.size())
		throw jive::compiler_error(jive::detail::strfmt("Argument error - expected ",
			operation().narguments(), ", received ", operands.size(), " arguments."));

	for (size_t n = 0; n < operation().narguments(); n++) {
		node::add_input(std::unique_ptr<jive::input>(
			new simple_input(this, operands[n], operation().argument(n))));
	}

	for (size_t n = 0; n < operation().nresults(); n++)
		node::add_output(std::unique_ptr<jive::output>(
			new simple_output(this, n, operation().result(n))));

	on_node_create(this);
}

jive::node *
simple_node::copy(jive::region * region, const std::vector<jive::output*> & operands) const
{
	auto node = create(region, *static_cast<const simple_op*>(&operation()), operands);
	graph()->mark_denormalized();
	return node;
}

jive::node *
simple_node::copy(jive::region * region, jive::substitution_map & smap) const
{
	std::vector<jive::output*> operands;
	for (size_t n = 0; n < ninputs(); n++) {
		auto operand = smap.lookup(input(n)->origin());
		operands.push_back(operand ? operand : input(n)->origin());
	}

	auto node = copy(region, operands);

	JIVE_DEBUG_ASSERT(node->noutputs() == noutputs());
	for (size_t n = 0; n < node->noutputs(); n++)
		smap.insert(output(n), node->output(n));

	return node;
}

}
