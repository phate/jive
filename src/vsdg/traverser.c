/*
 * Copyright 2010 2011 2012 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/common.h>
#include <jive/util/list.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/simple-node.h>
#include <jive/vsdg/tracker.h>
#include <jive/vsdg/traverser.h>
#include <jive/vsdg/type.h>

#include <string.h>

using namespace std::placeholders;

/* top down traverser */

namespace jive {

topdown_traverser::~topdown_traverser() noexcept {}

topdown_traverser::topdown_traverser(jive::region * region)
	: region_(region)
	, tracker_(region->graph())
{
	jive::node * node;
	JIVE_LIST_ITERATE(region->top_nodes, node, region_top_node_list) {
		tracker_.set_nodestate(node, traversal_nodestate::frontier);
	}

	for (size_t n = 0; n < region->narguments(); n++) {
		auto argument = region->argument(n);
		for (const auto & user : *argument) {
			if (!user->node() || !predecessors_visited(user->node()))
				continue;

			tracker_.set_nodestate(user->node(), traversal_nodestate::frontier);
		}
	}

	callbacks_.push_back(region->graph()->on_node_create.connect(
		std::bind(&topdown_traverser::node_create, this, _1)));
	callbacks_.push_back(region->graph()->on_input_change.connect(
		std::bind(&topdown_traverser::input_change, this, _1, _2, _3)));
}

bool
topdown_traverser::predecessors_visited(const jive::node * node) noexcept
{
	for (size_t n = 0; n < node->ninputs(); n++) {
		auto predecessor = node->input(n)->origin()->node();
		if (!predecessor)
			continue;

		if (tracker_.get_nodestate(predecessor) != traversal_nodestate::behind)
			return false;
	}

	return true;
}

jive::node *
topdown_traverser::next()
{
	jive::node * node = tracker_.peek_top();
	if (!node) return nullptr;

	tracker_.set_nodestate(node, traversal_nodestate::behind);
	for (size_t n = 0; n < node->noutputs(); n++) {
		for (const auto & user : *node->output(n)) {
			if (!user->node())
				continue;

			if (tracker_.get_nodestate(user->node()) == traversal_nodestate::ahead)
				tracker_.set_nodestate(user->node(), traversal_nodestate::frontier);
		}
	}

	return node;
}

void
topdown_traverser::node_create(jive::node * node)
{
	if (node->region() != region())
		return;

	if (predecessors_visited(node))
		tracker_.set_nodestate(node, traversal_nodestate::behind);
	else
		tracker_.set_nodestate(node, traversal_nodestate::frontier);
}

void
topdown_traverser::input_change(input * in, output * old_origin, output * new_origin)
{
	if (in->region() != region() || !in->node())
		return;

	auto state = tracker_.get_nodestate(in->node());
	
	/* ignore nodes that have been traversed already, or that are already
	marked for later traversal */
	if (state != traversal_nodestate::ahead)
		return;
	
	/* make sure node is visited eventually, might now be visited earlier
	as depth of the node could be lowered */
	tracker_.set_nodestate(in->node(), traversal_nodestate::frontier);
}

/* bottom up traverser */

bottomup_traverser::~bottomup_traverser() noexcept {}

bottomup_traverser::bottomup_traverser(jive::region * region, bool revisit)
	: region_(region)
	, tracker_(region->graph())
	, new_node_state_(revisit ? traversal_nodestate::frontier : traversal_nodestate::behind)
{
	jive::node * node;
	JIVE_LIST_ITERATE(region->bottom_nodes, node, region_bottom_list)
		tracker_.set_nodestate(node, traversal_nodestate::frontier);

	for (size_t n = 0; n < region->nresults(); n++) {
		auto node = region->result(n)->origin()->node();
		if (node && !node->has_successors())
			tracker_.set_nodestate(node, traversal_nodestate::frontier);
	}

	callbacks_.push_back(region->graph()->on_node_create.connect(
		std::bind(&bottomup_traverser::node_create, this, _1)));
	callbacks_.push_back(region->graph()->on_node_destroy.connect(
		std::bind(&bottomup_traverser::node_destroy, this, _1)));
	callbacks_.push_back(region->graph()->on_input_change.connect(
		std::bind(&bottomup_traverser::input_change, this, _1, _2, _3)));
}

jive::node *
bottomup_traverser::next()
{
	auto node = tracker_.peek_bottom();
	if (!node) return nullptr;

	tracker_.set_nodestate(node, traversal_nodestate::behind);
	for (size_t n = 0; n < node->ninputs(); n++) {
		auto producer = node->input(n)->origin()->node();
		if (producer && tracker_.get_nodestate(producer) == traversal_nodestate::ahead)
			tracker_.set_nodestate(producer, traversal_nodestate::frontier);
	}
	return node;
}

void
bottomup_traverser::node_create(jive::node * node)
{
	if (node->region() != region())
		return;

	tracker_.set_nodestate(node, new_node_state_);
}

void
bottomup_traverser::node_destroy(jive::node * node)
{
	if (node->region() != region())
		return;

	for (size_t n = 0; n < node->ninputs(); n++) {
		auto producer = node->input(n)->origin()->node();
		if (producer && tracker_.get_nodestate(producer) == traversal_nodestate::ahead)
			tracker_.set_nodestate(producer, traversal_nodestate::frontier);
	}
}

void
bottomup_traverser::input_change(input * in, output * old_origin, output * new_origin)
{
	if (in->region() != region() || !in->node())
		return;

	traversal_nodestate state = tracker_.get_nodestate(old_origin->node());
	
	/* ignore nodes that have been traversed already, or that are already
	marked for later traversal */
	if (state != traversal_nodestate::ahead)
		return;
	
	/* make sure node is visited eventually, might now be visited earlier
	as there (potentially) is one less obstructing node below */
	tracker_.set_nodestate(old_origin->node(), traversal_nodestate::frontier);
}

}
