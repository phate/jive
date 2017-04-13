/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/region.h>

#include <jive/common.h>

#include <jive/util/list.h>
#include <jive/vsdg/gate-interference-private.h>
#include <jive/vsdg/graph-private.h>
#include <jive/vsdg/simple_node.h>
#include <jive/vsdg/structural_node.h>
#include <jive/vsdg/substitution.h>
#include <jive/vsdg/traverser.h>

namespace jive {

/* argument */

argument::~argument() noexcept
{
	JIVE_DEBUG_ASSERT(users.empty());

	region()->graph()->on_oport_destroy(this);

	if (input())
		JIVE_LIST_REMOVE(input()->arguments, this, input_argument_list);

	if (gate()) {
		for (size_t n = 0; n < region()->narguments(); n++) {
			jive::argument * other = region()->argument(n);
			if (other == this || !other->gate())
				continue;
			jive_gate_interference_remove(region()->graph(), gate(), other->gate());
		}
	}

	for (size_t n = index()+1; n < region_->narguments(); n++)
		region_->argument(n)->set_index(n-1);
}

argument::argument(
	jive::region * region,
	size_t index,
	jive::structural_input * input,
	const jive::base::type & type)
	: oport(index)
	, region_(region)
	, input_(input)
	, type_(type.copy())
{
	input_argument_list.prev = input_argument_list.next = nullptr;
	if (input)
		JIVE_LIST_PUSH_BACK(input->arguments, this, input_argument_list);
}

argument::argument(
	jive::region * region,
	size_t index,
	jive::structural_input * input,
	jive::gate * gate)
	: oport(index, gate)
	, region_(region)
	, input_(input)
	, type_(gate->type().copy())
{
	input_argument_list.prev = input_argument_list.next = nullptr;
	if (input)
		JIVE_LIST_PUSH_BACK(input->arguments, this, input_argument_list);

	for (size_t n = 0; n < index; n++) {
		jive::argument * other = region->argument(n);
		if (!other->gate()) continue;
		jive_gate_interference_add(region->graph(), gate, other->gate());
	}
}

const jive::base::type &
argument::type() const noexcept
{
	return *type_;
}

jive::region *
argument::region() const noexcept
{
	return region_;
}

jive::node *
argument::node() const noexcept
{
	return nullptr;
}

/* result */

result::~result() noexcept
{
	region()->graph()->on_iport_destroy(this);

	origin()->users.erase(this);
	if (origin()->node() && !origin()->node()->has_users()) {
		JIVE_LIST_PUSH_BACK(origin()->node()->region()->bottom_nodes, origin()->node(),
			region_bottom_list);
	}

	if (output())
		JIVE_LIST_REMOVE(output()->results, this, output_result_list);

	if (gate()) {
		for (size_t n = 0; n < region()->nresults(); n++) {
			jive::result * other = region()->result(n);
			if (other == this || !other->gate())
				continue;
			jive_gate_interference_remove(region()->graph(), gate(), other->gate());
		}
	}

	for (size_t n = index()+1; n < region()->nresults(); n++)
		region()->result(n)->set_index(n-1);
}

result::result(
	jive::region * region,
	size_t index,
	jive::oport * origin,
	jive::structural_output * output,
	const jive::base::type & type)
	: iport(region, type, index, origin)
	, region_(region)
	, output_(output)
	, type_(type.copy())
{
	output_result_list.prev = output_result_list.next = nullptr;

	if (output)
		JIVE_LIST_PUSH_BACK(output->results, this, output_result_list);
}

result::result(
	jive::region * region,
	size_t index,
	jive::oport * origin,
	jive::structural_output * output,
	jive::gate * gate)
	: iport(region, gate->type(), index, origin, gate)
	, region_(region)
	, output_(output)
	, type_(gate->type().copy())
{
	output_result_list.prev = output_result_list.next = nullptr;

	if (output)
		JIVE_LIST_PUSH_BACK(output->results, this, output_result_list);

	for (size_t n = 0; n < index; n++) {
		jive::result * other = region->result(n);
		if (!other->gate()) continue;
		jive_gate_interference_add(region->graph(), gate, other->gate());
	}
}

const jive::base::type &
result::type() const noexcept
{
	return *type_;
}

jive::region *
result::region() const noexcept
{
	return region_;
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
region::add_argument(jive::structural_input * input, const jive::base::type & type)
{
	jive::argument * argument = new jive::argument(this, narguments(), input, type);
	arguments_.push_back(argument);

	graph()->on_oport_create(argument);

	return argument;
}

jive::argument *
region::add_argument(jive::structural_input * input, jive::gate * gate)
{
	jive::argument * argument = new jive::argument(this, narguments(), input, gate);
	arguments_.push_back(argument);

	graph()->on_oport_create(argument);

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
region::add_result(jive::oport * origin, structural_output * output, const base::type & type)
{
	jive::result * result = new jive::result(this, nresults(), origin, output, type);
	results_.push_back(result);

	if (origin->region() != this)
		throw jive::compiler_error("Invalid region result");

	graph()->on_iport_create(result);

	return result;
}

jive::result *
region::add_result(jive::oport * origin, structural_output * output, jive::gate * gate)
{
	jive::result * result = new jive::result(this, nresults(), origin, output, gate);
	results_.push_back(result);

	if (origin->region() != this)
		throw jive::compiler_error("Invalid region result");

	graph()->on_iport_create(result);

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
region::add_simple_node(const jive::operation & op, const std::vector<jive::oport*> & operands)
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
region::copy(region * target, substitution_map & smap) const
{
	std::function<void (
		const region*,
		region*,
		std::vector<std::vector<const jive::node*>>&,
		substitution_map&
	)>
	pre_copy_region = [&] (
		const region * source,
		region * target,
		std::vector<std::vector<const jive::node*>> & context,
		substitution_map & smap)
	{
		for (const auto & node : source->nodes) {
			if (node.depth() >= context.size())
				context.resize(node.depth()+1);
			context[node.depth()].push_back(&node);
		}
	};

	smap.insert(this, target);
	std::vector<std::vector<const jive::node*>> context;
	pre_copy_region(this, target, context, smap);

	/* copy arguments */
	for (size_t n = 0; n < narguments(); n++) {
		jive::argument * new_argument;
		if (argument(n)->gate()) {
			auto gate = argument(n)->gate();
			auto new_gate = smap.lookup(gate);
			if (!new_gate) {
				new_gate = graph()->create_gate(gate->type(), gate->name(), gate->rescls());
				smap.insert(gate, new_gate);
			}

			new_argument = target->add_argument(smap.lookup(argument(n)->input()), gate);
		} else {
			new_argument = target->add_argument(smap.lookup(argument(n)->input()), argument(n)->type());
		}
		smap.insert(argument(n), new_argument);
	}

	/* copy results */
	for (size_t n = 0; n < nresults(); n++) {
		auto origin = smap.lookup(result(n)->origin());
		if (!origin) origin = result(n)->origin();

		auto output = dynamic_cast<jive::structural_output*>(smap.lookup(result(n)->output()));
		if (result(n)->gate()) {
			auto gate = result(n)->gate();
			auto new_gate = smap.lookup(gate);
			if (!new_gate) {
				new_gate = graph()->create_gate(gate->type(), gate->name(), gate->rescls());
				smap.insert(gate, new_gate);
			}

			target->add_result(origin, output, gate);
		} else {
			target->add_result(origin, output, result(n)->type());
		}
	}

	/* copy nodes */
	for (size_t n = 0; n < context.size(); n++) {
		for (const auto node : context[n]) {
			target = smap.lookup(node->region());
			node->copy(target, smap);
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
