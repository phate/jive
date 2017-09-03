/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/region.h>

#include <jive/common.h>

#include <jive/util/list.h>
#include <jive/vsdg/graph-private.h>
#include <jive/vsdg/simple_node.h>
#include <jive/vsdg/structural_node.h>
#include <jive/vsdg/substitution.h>
#include <jive/vsdg/traverser.h>

namespace jive {

/* argument */

argument::~argument() noexcept
{
	region()->graph()->on_output_destroy(this);

	if (input())
		JIVE_LIST_REMOVE(input()->arguments, this, input_argument_list);

	if (port().gate())
		port().gate()->clear_interferences();

	for (size_t n = index()+1; n < region()->narguments(); n++)
		region()->argument(n)->set_index(n-1);
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
	region()->graph()->on_input_destroy(this);

	if (output())
		JIVE_LIST_REMOVE(output()->results, this, output_result_list);

	if (port().gate())
		port().gate()->clear_interferences();

	for (size_t n = index()+1; n < region()->nresults(); n++)
		region()->result(n)->set_index(n-1);
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
	graph_->on_region_destroy(this);

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
	graph->on_region_create(this);
}

region::region(jive::structural_node * node)
	: graph_(node->graph())
	, node_(node)
{
	top_nodes.first = top_nodes.last = nullptr;
	bottom_nodes.first = bottom_nodes.last = nullptr;
	graph()->on_region_create(this);
}

jive::argument *
region::add_argument(jive::structural_input * input, const jive::port & port)
{
	jive::argument * argument = new jive::argument(this, narguments(), input, port);
	arguments_.push_back(argument);

	graph()->on_output_create(argument);

	return argument;
}

void
region::remove_argument(size_t index)
{
	JIVE_DEBUG_ASSERT(index < narguments());
	jive::argument * argument = arguments_[index];

	delete argument;
	for (size_t n = index; n < arguments_.size()-1; n++) {
		JIVE_DEBUG_ASSERT(arguments_[n+1]->index() == n);
		arguments_[n] = arguments_[n+1];
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

	graph()->on_input_create(result);

	return result;
}

void
region::remove_result(size_t index)
{
	JIVE_DEBUG_ASSERT(index < results_.size());
	jive::result * result = results_[index];

	delete result;
	for (size_t n = index; n < results_.size()-1; n++) {
		JIVE_DEBUG_ASSERT(results_[n+1]->index() == n);
		results_[n] = results_[n+1];
	}
	results_.pop_back();
}

jive::simple_node *
region::add_simple_node(const jive::simple_op & op, const std::vector<jive::output*> & operands)
{
	return new jive::simple_node(op, this, operands);
}

jive::structural_node *
region::add_structural_node(const jive::operation & op, size_t nsubregions)
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
	std::vector<std::vector<const jive::node*>> context;
	for (const auto & node : nodes) {
		if (node.depth() >= context.size())
			context.resize(node.depth()+1);
		context[node.depth()].push_back(&node);
	}

	/* copy arguments */
	if (copy_arguments) {
		for (size_t n = 0; n < narguments(); n++) {
			jive::argument * new_argument;
			if (argument(n)->port().gate()) {
				auto gate = argument(n)->port().gate();
				auto new_gate = smap.lookup(gate);
				if (!new_gate) {
					new_gate = graph()->create_gate(gate);
					smap.insert(gate, new_gate);
				}

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
				if (!new_gate) {
					new_gate = graph()->create_gate(gate);
					smap.insert(gate, new_gate);
				}

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

}	//namespace

#ifdef JIVE_DEBUG
void
jive_region_verify_top_node_list(struct jive::region * region)
{
	/* check whether all nodes in the top_node_list are really nullary nodes */
	jive::node * node;
	JIVE_LIST_ITERATE(region->top_nodes, node, region_top_node_list)
		JIVE_DEBUG_ASSERT(node->ninputs() == 0);

	/* check whether all nullary nodes from the region are in the top_node_list */
	for (const auto & node : region->nodes) {
		if (node.ninputs() != 0)
			continue;

		jive::node * top;
		JIVE_LIST_ITERATE(region->top_nodes, top, region_top_node_list) {
			if (top == &node)
				break;
		}
		if (top == NULL)
			JIVE_DEBUG_ASSERT(0);
	}
}
#endif
