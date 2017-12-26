/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/common.h>

#include <jive/rvsdg/graph.h>
#include <jive/rvsdg/notifiers.h>
#include <jive/rvsdg/simple-node.h>
#include <jive/rvsdg/region.h>
#include <jive/rvsdg/substitution.h>
#include <jive/rvsdg/traverser.h>
#include <jive/util/list.h>

namespace jive {

/* argument */

argument::~argument() noexcept
{
	on_output_destroy(this);

	if (input())
		JIVE_LIST_REMOVE(input()->arguments, this, input_argument_list);

	if (port().gate())
		port().gate()->clear_interferences();
}

argument::argument(
	jive::region * region,
	size_t index,
	jive::structural_input * input,
	const jive::port & port)
	: output(index, region, port)
	, input_(input)
{
	input_argument_list.prev = input_argument_list.next = nullptr;
	if (input)
		JIVE_LIST_PUSH_BACK(input->arguments, this, input_argument_list);

	if (port.gate()) {
		for (size_t n = 0; n < index; n++) {
			auto other = region->argument(n);
			if (!other->port().gate()) continue;
			port.gate()->add_interference(other->port().gate());
		}
	}
}

jive::node *
argument::node() const noexcept
{
	return nullptr;
}

/* result */

result::~result() noexcept
{
	on_input_destroy(this);

	if (output())
		JIVE_LIST_REMOVE(output()->results, this, output_result_list);

	if (port().gate())
		port().gate()->clear_interferences();
}

result::result(
	jive::region * region,
	size_t index,
	jive::output * origin,
	jive::structural_output * output,
	const jive::port & port)
	: input(index, origin, region, port)
	, output_(output)
{
	output_result_list.prev = output_result_list.next = nullptr;

	if (output)
		JIVE_LIST_PUSH_BACK(output->results, this, output_result_list);

	if (port.gate()) {
		for (size_t n = 0; n < index; n++) {
			auto other = region->result(n);
			if (!other->port().gate()) continue;
			port.gate()->add_interference(other->port().gate());
		}
	}
}

jive::node *
result::node() const noexcept
{
	return nullptr;
}

/* region */

region::~region()
{
	on_region_destroy(this);

	while (results_.size())
		remove_result(results_.size()-1);

	prune(false);
	JIVE_DEBUG_ASSERT(nodes.empty());
	JIVE_DEBUG_ASSERT(top_nodes.first == nullptr && top_nodes.last == nullptr);
	JIVE_DEBUG_ASSERT(bottom_nodes.first == nullptr && bottom_nodes.last == nullptr);

	while (arguments_.size())
		remove_argument(arguments_.size()-1);
}

region::region(jive::region * parent, jive::graph * graph)
	: graph_(graph)
	, node_(nullptr)
{
	top_nodes.first = top_nodes.last = nullptr;
	bottom_nodes.first = bottom_nodes.last = nullptr;
	on_region_create(this);
}

region::region(jive::structural_node * node)
	: graph_(node->graph())
	, node_(node)
{
	top_nodes.first = top_nodes.last = nullptr;
	bottom_nodes.first = bottom_nodes.last = nullptr;
	on_region_create(this);
}

jive::argument *
region::add_argument(jive::structural_input * input, const jive::port & port)
{
	jive::argument * argument = new jive::argument(this, narguments(), input, port);
	arguments_.push_back(argument);

	on_output_create(argument);

	return argument;
}

void
region::remove_argument(size_t index)
{
	JIVE_DEBUG_ASSERT(index < narguments());
	jive::argument * argument = arguments_[index];

	delete argument;
	for (size_t n = index; n < arguments_.size()-1; n++) {
		arguments_[n] = arguments_[n+1];
		arguments_[n]->index_ = n;
	}
	arguments_.pop_back();
}

jive::result *
region::add_result(jive::output * origin, structural_output * output, const jive::port & port)
{
	jive::result * result = new jive::result(this, nresults(), origin, output, port);
	results_.push_back(result);

	if (origin->region() != this)
		throw jive::compiler_error("Invalid region result");

	on_input_create(result);

	return result;
}

void
region::remove_result(size_t index)
{
	JIVE_DEBUG_ASSERT(index < results_.size());
	jive::result * result = results_[index];

	delete result;
	for (size_t n = index; n < results_.size()-1; n++) {
		results_[n] = results_[n+1];
		results_[n]->index_ = n;
	}
	results_.pop_back();
}

jive::simple_node *
region::add_simple_node(const jive::simple_op & op, const std::vector<jive::output*> & operands)
{
	return new jive::simple_node(op, this, operands);
}

jive::structural_node *
region::add_structural_node(const jive::structural_op & op, size_t nsubregions)
{
	return new jive::structural_node(op, this, nsubregions);
}

void
region::remove_node(jive::node * node)
{
	delete node;
}

void
region::copy(
	region * target,
	substitution_map & smap,
	bool copy_arguments,
	bool copy_results) const
{
	smap.insert(this, target);

	/* order nodes top-down */
	std::vector<std::vector<const jive::node*>> context(nnodes());
	for (const auto & node : nodes)
		context[node.depth()].push_back(&node);

	/* copy arguments */
	if (copy_arguments) {
		for (size_t n = 0; n < narguments(); n++) {
			jive::argument * new_argument;
			if (argument(n)->port().gate()) {
				auto gate = argument(n)->port().gate();
				auto new_gate = smap.lookup(gate);
				if (!new_gate)
					smap.insert(gate, gate::create(graph(), gate));

				new_argument = target->add_argument(smap.lookup(argument(n)->input()), gate);
			} else {
				new_argument = target->add_argument(smap.lookup(argument(n)->input()), argument(n)->type());
			}
			smap.insert(argument(n), new_argument);
		}
	}

	/* copy nodes */
	for (size_t n = 0; n < context.size(); n++) {
		for (const auto node : context[n]) {
			target = smap.lookup(node->region());
			node->copy(target, smap);
		}
	}

	/* copy results */
	if (copy_results) {
		for (size_t n = 0; n < nresults(); n++) {
			auto origin = smap.lookup(result(n)->origin());
			if (!origin) origin = result(n)->origin();

			auto output = dynamic_cast<jive::structural_output*>(smap.lookup(result(n)->output()));
			if (result(n)->port().gate()) {
				auto gate = result(n)->port().gate();
				auto new_gate = smap.lookup(gate);
				if (!new_gate)
					smap.insert(gate, gate::create(graph(), gate));

				target->add_result(origin, output, gate);
			} else {
				target->add_result(origin, output, result(n)->type());
			}
		}
	}
}

void
region::prune(bool recursive)
{
	while (bottom_nodes.first)
		remove_node(bottom_nodes.first);

	if (!recursive)
		return;

	for (const auto & node : nodes) {
		if (auto snode = dynamic_cast<const jive::structural_node*>(&node)) {
			for (size_t n = 0; n < snode->nsubregions(); n++)
				snode->subregion(n)->prune(recursive);
		}
	}
}

void
region::normalize(bool recursive)
{
	for (auto node : jive::topdown_traverser(this)) {
		if (auto structnode = dynamic_cast<const jive::structural_node*>(node)) {
			for (size_t n = 0; n < structnode->nsubregions(); n++)
				structnode->subregion(n)->normalize(recursive);
		}

		graph()->node_normal_form(typeid(node->operation()))->normalize_node(node);
	}
}

}	//namespace
