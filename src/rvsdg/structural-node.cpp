/*
 * Copyright 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/rvsdg/graph.hpp>
#include <jive/rvsdg/notifiers.hpp>
#include <jive/rvsdg/region.hpp>
#include <jive/rvsdg/structural-node.hpp>
#include <jive/rvsdg/substitution.hpp>

namespace jive {

/* structural input */

structural_input::~structural_input() noexcept
{
	JIVE_DEBUG_ASSERT(arguments.empty());

	on_input_destroy(this);
}

structural_input::structural_input(
	jive::structural_node * node,
	jive::output * origin,
	const jive::port & port)
: input(origin, node->region(), port)
, node_(node)
{
	on_input_create(this);
}

jive::structural_node *
structural_input::node() const noexcept
{
	return node_;
}

/* structural output */

structural_output::~structural_output() noexcept
{
	JIVE_DEBUG_ASSERT(results.empty());

	on_output_destroy(this);
}

structural_output::structural_output(
	jive::structural_node * node,
	const jive::port & port)
: output(node->region(), port)
, node_(node)
{
	on_output_create(this);
}

jive::structural_node *
structural_output::node() const noexcept
{
	return node_;
}

/* structural node */

structural_node::~structural_node()
{
	on_node_destroy(this);

	subregions_.clear();
}

structural_node::structural_node(
	const jive::structural_op & op,
	jive::region * region,
	size_t nsubregions)
	: node(op.copy(), region)
{
	/* FIXME: check that nsubregions is unequal zero */
	for (size_t n = 0; n < nsubregions; n++)
		subregions_.emplace_back(std::unique_ptr<jive::region>(new jive::region(this)));

	on_node_create(this);
}

structural_input *
structural_node::append_input(std::unique_ptr<structural_input> input)
{
	if (input->node() != this)
		throw compiler_error("Appending input to wrong node.");

	auto index = input->index();
	JIVE_DEBUG_ASSERT(index == 0);
	if (index != 0
	|| (index == 0 && ninputs() > 0 && this->input(0) == input.get()))
		return this->input(index);

	auto sinput = std::unique_ptr<jive::input>(input.release());
	return static_cast<structural_input*>(node::add_input(std::move(sinput)));
}

structural_output *
structural_node::append_output(std::unique_ptr<structural_output> output)
{
	if (output->node() != this)
		throw compiler_error("Appending output to wrong node.");

	auto index = output->index();
	JIVE_DEBUG_ASSERT(index == 0);
	if (index != 0
	|| (index == 0 && noutputs() > 0 && this->output(0) == output.get()))
		return this->output(index);

	auto soutput = std::unique_ptr<jive::output>(output.release());
	return static_cast<structural_output*>(node::add_output(std::move(soutput)));
}

jive::structural_node *
structural_node::copy(jive::region * region, const std::vector<jive::output*> & operands) const
{
	jive::substitution_map smap;

	size_t noperands = std::min(operands.size(), ninputs());
	for (size_t n = 0; n < noperands; n++)
		smap.insert(input(n)->origin(), operands[n]);

	return copy(region, smap);
}

jive::structural_node *
structural_node::copy(jive::region * region, jive::substitution_map & smap) const
{
	graph()->mark_denormalized();
	auto node = new structural_node(*static_cast<const structural_op*>(&operation()), region, 0);

	/* copy inputs */
	for (size_t n = 0; n < ninputs(); n++) {
		auto origin = smap.lookup(input(n)->origin());
		if (!origin) origin = input(n)->origin();

		auto new_input = structural_input::create(node, origin, input(n)->port());
		smap.insert(input(n), new_input);
	}

	/* copy outputs */
	for (size_t n = 0; n < noutputs(); n++) {
		auto new_output = structural_output::create(node, output(n)->port());
		smap.insert(output(n), new_output);
	}

	/* copy regions */
	for (size_t n = 0; n < nsubregions(); n++) {
		node->subregions_.emplace_back(std::make_unique<jive::region>(node));
		subregion(n)->copy(node->subregions_.back().get(), smap, true, true);
	}

	return node;
}

}
