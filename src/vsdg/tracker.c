/*
 * Copyright 2010 2011 2012 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/common.h>
#include <jive/vsdg/graph-private.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/simple_node.h>
#include <jive/vsdg/tracker.h>

using namespace std::placeholders;

namespace jive {

/* tracker */

tracker::~tracker()
{
	jive_graph_return_tracker_slot(graph_, slot_);
}

tracker::tracker(jive::graph * graph, size_t nstates)
	: graph_(graph)
	, slot_(jive_graph_reserve_tracker_slot(graph_))
	, states_(nstates)
{
	for (size_t n = 0; n < states_.size(); n++)
		states_[n]= std::make_unique<tracker_depth_state>();

	depth_callback_ = graph->on_node_depth_change.connect(
		std::bind(&tracker::node_depth_change, this, _1, _2));
	destroy_callback_ = graph->on_node_destroy.connect(
		std::bind(&tracker::node_destroy, this, _1));
}

jive_tracker_nodestate*
tracker::map_node(jive::node * node)
{
	return jive_node_get_tracker_state(node, slot_);
}

void
tracker::node_depth_change(jive::node * node, size_t old_depth)
{
	jive_tracker_nodestate * nodestate = map_node(node);
	if (nodestate->state >= states_.size()) {
		return;
	}
	states_[nodestate->state]->remove(nodestate, old_depth);
	states_[nodestate->state]->add(nodestate, node->depth());
}

void
tracker::node_destroy(jive::node * node)
{
	jive_tracker_nodestate * nodestate = map_node(node);
	if (nodestate->state >= states_.size()) {
		return;
	}
	states_[nodestate->state]->remove(nodestate, node->depth());
}

ssize_t
tracker::get_nodestate(jive::node * node)
{
	return map_node(node)->state;
}

void
tracker::set_nodestate(jive::node * node, size_t state)
{
	jive_tracker_nodestate * nodestate = map_node(node);
	
	if (nodestate->state != state) {
		if (nodestate->state < states_.size())
			states_[nodestate->state]->remove(nodestate, node->depth());

		nodestate->state = state;
		if (nodestate->state < states_.size())
			states_[nodestate->state]->add(nodestate, node->depth());
	}
}

jive::node *
tracker::peek_top(size_t state) const
{
	auto nodestate = states_[state]->pop_top();
	return nodestate ? nodestate->node : nullptr;
}

jive::node *
tracker::peek_bottom(size_t state) const
{
	auto nodestate = states_[state]->pop_bottom();
	return nodestate ? nodestate->node : nullptr;
}

computation_tracker::computation_tracker(jive::graph * graph)
	: graph_(graph)
	, slot_(jive_graph_reserve_tracker_slot(graph))
	, nodestates_(std::make_unique<tracker_depth_state>())
{
}

computation_tracker::~computation_tracker() noexcept
{
	jive_graph_return_tracker_slot(graph_, slot_);
}

jive_tracker_nodestate *
computation_tracker::map_node(jive::node * node)
{
	return jive_node_get_tracker_state(node, slot_);
}

void
computation_tracker::invalidate(jive::node * node)
{
	jive_tracker_nodestate * nodestate = map_node(node);
	if (nodestate->state == jive_tracker_nodestate_none) {
		nodestates_->add(nodestate, node->depth());
		nodestate->state = 0;
	}
}

void
computation_tracker::invalidate_below(jive::node * node)
{
	for (size_t n = 0; n < node->noutputs(); n++) {
		for (const auto & user : *node->output(n))
			invalidate(user->node());
	}
}

jive::node *
computation_tracker::pop_top()
{
	auto nodestate = nodestates_->pop_top();
	return nodestate ? nodestate->node : nullptr;
}

}
