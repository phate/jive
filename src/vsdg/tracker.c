/*
 * Copyright 2010 2011 2012 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/common.h>
#include <jive/vsdg/graph-private.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/tracker-private.h>
#include <jive/vsdg/tracker.h>

using namespace std::placeholders;

namespace jive {

/* tracker */

tracker::~tracker()
{
	for (size_t n = 0; n < states_.size(); n++) {
		jive_graph_return_tracker_depth_state(graph_, states_[n]);
	}
	jive_graph_return_tracker_slot(graph_, slot_);
}

tracker::tracker(jive_graph * graph, size_t nstates)
	: graph_(graph)
	, slot_(jive_graph_reserve_tracker_slot(graph_))
	, states_(nstates, nullptr)
{
	for (size_t n = 0; n < states_.size(); n++) {
		states_[n] = jive_graph_reserve_tracker_depth_state(graph);
	}

	depth_callback_ = graph->on_node_depth_change.connect(
		std::bind(&tracker::node_depth_change, this, _1, _2));
	destroy_callback_ = graph->on_node_destroy.connect(
		std::bind(&tracker::node_destroy, this, _1));
}

jive_tracker_nodestate*
tracker::map_node(jive_node * node)
{
	return jive_node_get_tracker_state(node, slot_);
}

void
tracker::node_depth_change(jive_node * node, size_t old_depth)
{
	jive_tracker_nodestate * nodestate = map_node(node);
	if (nodestate->state >= states_.size()) {
		return;
	}
	jive_tracker_depth_state_remove(states_[nodestate->state], nodestate, old_depth);
	jive_tracker_depth_state_add(states_[nodestate->state], nodestate, node->depth_from_root);
	
}

void
tracker::node_destroy(jive_node * node)
{
	jive_tracker_nodestate * nodestate = map_node(node);
	if (nodestate->state >= states_.size()) {
		return;
	}
	jive_tracker_depth_state_remove(states_[nodestate->state], nodestate, node->depth_from_root);
}

ssize_t
tracker::get_nodestate(jive_node * node)
{
	return map_node(node)->state;
}

void
tracker::set_nodestate(jive_node * node, size_t state)
{
	jive_tracker_nodestate * nodestate = map_node(node);
	
	if (nodestate->state != state) {
		if (nodestate->state < states_.size()) {
			jive_tracker_depth_state_remove(
				states_[nodestate->state], nodestate,
				node->depth_from_root);
		}
		nodestate->state = state;
		if (nodestate->state < states_.size()) {
			jive_tracker_depth_state_add(
				states_[nodestate->state], nodestate,
				node->depth_from_root);
		}
	}
}

jive_node *
tracker::peek_top(size_t state) const
{
	jive_tracker_nodestate * nodestate = jive_tracker_depth_state_pop_top(states_[state]);
	return nodestate ? nodestate->node : nullptr;
}

jive_node *
tracker::peek_bottom(size_t state) const
{
	jive_tracker_nodestate * nodestate = jive_tracker_depth_state_pop_bottom(states_[state]);
	return nodestate ? nodestate->node : nullptr;
}

computation_tracker::computation_tracker(jive_graph * graph)
	: graph_(graph)
	, slot_(jive_graph_reserve_tracker_slot(graph))
	, nodestates_(jive_graph_reserve_tracker_depth_state(graph))
{
}

computation_tracker::~computation_tracker() noexcept
{
	jive_graph_return_tracker_depth_state(graph_, nodestates_);
	jive_graph_return_tracker_slot(graph_, slot_);
}

jive_tracker_nodestate *
computation_tracker::map_node(jive_node * node)
{
	return jive_node_get_tracker_state(node, slot_);
}

void
computation_tracker::invalidate(jive_node * node)
{
	jive_tracker_nodestate * nodestate = map_node(node);
	if (nodestate->state == jive_tracker_nodestate_none) {
		jive_tracker_depth_state_add(nodestates_, nodestate, node->depth_from_root);
		nodestate->state = 0;
	}
}

void
computation_tracker::invalidate_below(jive_node * node)
{
	for (size_t n = 0; n < node->noutputs; n++) {
		output * out = node->outputs[n];
		input * user;
		JIVE_LIST_ITERATE(out->users, user, output_users_list) {
			invalidate(user->node());
		}
	}
}

jive_node *
computation_tracker::pop_top()
{
	jive_tracker_nodestate * nodestate = jive_tracker_depth_state_pop_top(nodestates_);
	return nodestate ? nodestate->node : nullptr;
}

}
