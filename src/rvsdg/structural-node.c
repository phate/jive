/*
 * Copyright 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/rvsdg/graph.h>
#include <jive/rvsdg/notifiers.h>
#include <jive/rvsdg/region.h>
#include <jive/rvsdg/structural-node.h>
#include <jive/rvsdg/substitution.h>

namespace jive {

/* structural input */

structural_input::~structural_input()
{
	JIVE_DEBUG_ASSERT(arguments.first == nullptr && arguments.last == nullptr);

	on_input_destroy(this);

	if (port().gate())
		port().gate()->clear_interferences();
}

structural_input::structural_input(
	jive::structural_node * node,
	size_t index,
	jive::output * origin,
	const jive::port & port)
	: input(index, origin, node->region(), port)
	, node_(node)
{
	arguments.first = arguments.last = nullptr;

	if (port.gate()) {
		for (size_t n = 0; n < index; n++) {
			auto other = node->input(n);
			if (!other->port().gate()) continue;
			port.gate()->add_interference(other->port().gate());
		}
	}

	on_input_create(this);
}

jive::structural_node *
structural_input::node() const noexcept
{
	return node_;
}

/* structural output */

structural_output::~structural_output()
{
	JIVE_DEBUG_ASSERT(results.first == nullptr && results.last == nullptr);

	on_output_destroy(this);

	if (port().gate())
		port().gate()->clear_interferences();
}

structural_output::structural_output(
	jive::structural_node * node,
	size_t index,
	const jive::port & port)
	: output(index, node->region(), port)
	, node_(node)
{
	results.first = results.last = nullptr;

	if (port.gate()) {
		for (size_t n = 0; n < index; n++) {
			auto other = node->output(n);
			if (!other->port().gate()) continue;
			port.gate()->add_interference(other->port().gate());
		}
	}

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

jive::structural_input *
structural_node::add_input(const jive::port & port, jive::output * origin)
{
	node::add_input(std::unique_ptr<jive::input>(
		new structural_input(this, ninputs(), origin, port)));
	return input(ninputs()-1);
}

jive::structural_output *
structural_node::add_output(const jive::port & port)
{
	node::add_output(std::unique_ptr<structural_output>(
		new structural_output(this, noutputs(), port)));
	return output(noutputs()-1);
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
	auto node = region->add_structural_node(*static_cast<const structural_op*>(&operation()), 0);

	/* copy inputs */
	for (size_t n = 0; n < ninputs(); n++) {
		auto origin = smap.lookup(input(n)->origin());
		if (!origin) origin = input(n)->origin();

		jive::structural_input * new_input;
		if (input(n)->port().gate()) {
			auto gate = input(n)->port().gate();
			auto new_gate = smap.lookup(gate);
			if (!new_gate)
				smap.insert(gate, gate::create(graph(), gate));

			new_input = node->add_input(new_gate, origin);
		} else {
			new_input = node->add_input(input(n)->port(), origin);
		}
		smap.insert(input(n), new_input);
	}

	/* copy outputs */
	for (size_t n = 0; n < noutputs(); n++) {
		jive::structural_output * new_output;
		if (output(n)->port().gate()) {
			auto gate = output(n)->port().gate();
			auto new_gate = smap.lookup(gate);
			if (!new_gate)
				smap.insert(gate, gate::create(graph(), gate));

			new_output = node->add_output(new_gate);
		} else {
			new_output = node->add_output(output(n)->port());
		}
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
