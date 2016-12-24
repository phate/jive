/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/region.h>

#include <jive/common.h>

#include <jive/util/list.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/gate-interference-private.h>
#include <jive/vsdg/graph-private.h>
#include <jive/vsdg/substitution.h>
#include <jive/vsdg/traverser.h>

namespace jive {

/* argument */

argument::~argument() noexcept
{
	JIVE_DEBUG_ASSERT(users.empty());

	region()->graph()->on_argument_destroy(this);

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

/* region */

region::~region()
{
	graph_->on_region_destroy(this);

	std::vector<std::vector<const jive::node*>> sorted_nodes;
	for (const auto & node : nodes)	{
		if (node.depth() >= sorted_nodes.size())
			sorted_nodes.resize(node.depth()+1);
		sorted_nodes[node.depth()].push_back(&node);
	}
	for (auto it = sorted_nodes.rbegin(); it != sorted_nodes.rend(); it++) {
		for (const auto & node : *it) {
			std::vector<jive::region*> subregions;
			if (dynamic_cast<const jive::region_anchor_op*>(&node->operation())) {
				for (size_t n = 0; n < node->ninputs(); n++) {
					auto tail = node->input(n)->origin()->node();
					if (tail && tail == tail->region()->bottom())
						subregions.push_back(tail->region());
				}
			}

			delete node;
			for (size_t n = 0; n < subregions.size(); n++)
				delete subregions[n];
		}
	}

	/*
		FIXME: some unit tests create regions without anchor nodes
	*/
	jive::region * subregion, * next;
	JIVE_LIST_ITERATE_SAFE(subregions, subregion, next, region_subregions_list) {
		delete subregion;
	}

	JIVE_DEBUG_ASSERT(nodes.empty());
	JIVE_DEBUG_ASSERT(subregions.first == nullptr && subregions.last == nullptr);

	while (arguments_.size())
		remove_argument(arguments_.size()-1);

	if (parent_)
		JIVE_LIST_REMOVE(parent_->subregions, this, region_subregions_list);

	while (results_.size())
		remove_result(results_.size()-1);
}

region::region(jive::region * parent, jive_graph * graph)
	: depth_(0)
	, top_(nullptr)
	, bottom_(nullptr)
	, graph_(graph)
	, parent_(parent)
	, anchor_(nullptr)
{
	top_nodes.first = top_nodes.last = nullptr;
	subregions.first = subregions.last = nullptr;
	region_subregions_list.prev = region_subregions_list.next = nullptr;

	if (parent_) {
		JIVE_LIST_PUSH_BACK(parent_->subregions, this, region_subregions_list);
		depth_ = parent_->depth() + 1;
	}

	graph->on_region_create(this);
}

jive::argument *
region::add_argument(jive::structural_input * input, const jive::base::type & type)
{
	jive::argument * argument = new jive::argument(this, narguments(), input, type);
	arguments_.push_back(argument);

	graph()->on_argument_create(argument);

	return argument;
}

jive::argument *
region::add_argument(jive::structural_input * input, jive::gate * gate)
{
	jive::argument * argument = new jive::argument(this, narguments(), input, gate);
	arguments_.push_back(argument);

	graph()->on_argument_create(argument);

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

void
region::reparent(jive::region * new_parent) noexcept
{
	std::function<void (jive::region *, size_t)> set_depth_recursive = [&](
		jive::region * region,
		size_t new_depth)
	{
		region->depth_ = new_depth;
		jive::region * subregion;
		JIVE_LIST_ITERATE(region->subregions, subregion, region_subregions_list)
			set_depth_recursive(subregion, new_depth + 1);
	};

	JIVE_LIST_REMOVE(parent()->subregions, this, region_subregions_list);

	parent_ = new_parent;
	size_t new_depth = new_parent->depth() + 1;
	if (new_depth != depth())
		set_depth_recursive(this, new_depth);

	JIVE_LIST_PUSH_BACK(parent_->subregions, this, region_subregions_list);
}

bool
region::contains(const jive::node * node) const noexcept
{
	const jive::region * tmp = node->region();
	while (tmp->depth() >= depth()) {
		if (tmp == this)
			return true;
		tmp = tmp->parent();
		if (!tmp)
			break;
	}
	return false;
}

void
region::copy(region * target, substitution_map & smap, bool copy_top, bool copy_bottom) const
{
	std::function<void (
		const region*,
		region*,
		std::vector<std::vector<const jive::node*>>&,
		substitution_map&,
		bool,
		bool
	)>
	pre_copy_region = [&] (
		const region * source,
		region * target,
		std::vector<std::vector<const jive::node*>> & context,
		substitution_map & smap,
		bool copy_top,
		bool copy_bottom)
	{
		for (const auto & node : source->nodes) {
			if (!copy_top && &node == source->top())
				continue;
			if (!copy_bottom && &node == source->bottom())
				continue;
	
			if (node.depth() >= context.size())
				context.resize(node.depth()+1);
			context[node.depth()].push_back(&node);
		}

		jive::region * subregion;
		JIVE_LIST_ITERATE(source->subregions, subregion, region_subregions_list) {
			jive::region * target_subregion = new jive::region(target, target->graph());
			smap.insert(subregion, target_subregion);
			pre_copy_region(subregion, target_subregion, context, smap, true, true);
		}
	};

	smap.insert(this, target);
	std::vector<std::vector<const jive::node*>> context;
	pre_copy_region(this, target, context, smap, copy_top, copy_bottom);

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

	for (size_t n = 0; n < context.size(); n++) {
		for (const auto node : context[n]) {
			target = smap.lookup(node->region());
			jive::node * new_node = node->copy(target, smap);
			if (node->region()->top() == node)
				target->set_top(new_node);
			if (node->region()->bottom() == node)
				target->set_bottom(new_node);
		}
	}
}

}	//namespace

#ifdef JIVE_DEBUG
void
jive_region_verify_top_node_list(struct jive::region * region)
{
	jive::region * subregion;
	JIVE_LIST_ITERATE(region->subregions, subregion, region_subregions_list)
		jive_region_verify_top_node_list(subregion);

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
