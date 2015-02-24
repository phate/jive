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

static jive_tracker_nodestate *
jive_tracker_map_node(jive_tracker * self, jive_node * node)
{
	return jive_node_get_tracker_state(node, self->slot_);
}

static void
node_depth_change(jive_tracker * self, jive_node * node, size_t old_depth)
{
	jive_tracker_nodestate * nodestate = jive_tracker_map_node(self, node);
	if (nodestate->state >= self->states_.size()) {
		return;
	}
	jive_tracker_depth_state_remove(self->states_[nodestate->state], nodestate, old_depth);
	jive_tracker_depth_state_add(self->states_[nodestate->state], nodestate, node->depth_from_root);
	
}

static void
node_destroy(jive_tracker * self, jive_node * node)
{
	jive_tracker_nodestate * nodestate = jive_tracker_map_node(self, node);
	if (nodestate->state >= self->states_.size()) {
		return;
	}
	jive_tracker_depth_state_remove(self->states_[nodestate->state], nodestate, node->depth_from_root);
}

jive_tracker::jive_tracker(jive_graph * graph, size_t nstates)
	: graph_(graph)
	, slot_(jive_graph_reserve_tracker_slot(graph_))
	, states_(nstates, nullptr)
{
	for (size_t n = 0; n < states_.size(); n++) {
		states_[n] = jive_graph_reserve_tracker_depth_state(graph);
	}

	depth_callback_ = graph->on_node_depth_change.connect(
		std::bind(node_depth_change, this, _1, _2));
	destroy_callback_ = graph->on_node_destroy.connect(
		std::bind(node_destroy, this, _1));
}

jive_tracker::~jive_tracker()
{
	for (size_t n = 0; n < states_.size(); n++) {
		jive_graph_return_tracker_depth_state(graph_, states_[n]);
	}
	jive_graph_return_tracker_slot(graph_, slot_);
}

int
jive_tracker_get_nodestate(jive_tracker * self, jive_node * node)
{
	return jive_tracker_map_node(self, node)->state;
}

int
jive_tracker_get_nodetag(jive_tracker * self, jive_node * node)
{
	return jive_tracker_map_node(self, node)->tag;
}

void
jive_tracker_set_nodestate(jive_tracker * self, jive_node * node, size_t state, int tag)
{
	jive_tracker_nodestate * nodestate = jive_tracker_map_node(self, node);
	nodestate->tag = tag;
	
	if (nodestate->state != state) {
		if (nodestate->state < self->states_.size()) {
			jive_tracker_depth_state_remove(self->states_[nodestate->state], nodestate,
				node->depth_from_root);
		}
		nodestate->state = state;
		if (nodestate->state < self->states_.size()) {
			jive_tracker_depth_state_add(self->states_[nodestate->state], nodestate,
				node->depth_from_root);
		}
	}
}

jive_node *
jive_tracker_pop_top(jive_tracker * self, size_t state)
{
	jive_tracker_nodestate * nodestate = jive_tracker_depth_state_pop_top(self->states_[state]);
	return nodestate ? nodestate->node : nullptr;
}

jive_node *
jive_tracker_pop_bottom(jive_tracker * self, size_t state)
{
	jive_tracker_nodestate * nodestate = jive_tracker_depth_state_pop_bottom(self->states_[state]);
	return nodestate ? nodestate->node : nullptr;
}

jive_traversal_tracker::jive_traversal_tracker(jive_graph * graph)
	: jive_tracker(graph, 2)
{
}

jive_traversal_nodestate
jive_traversal_tracker_get_nodestate(jive_traversal_tracker * self, jive_node * node)
{
	return (jive_traversal_nodestate) jive_tracker_get_nodestate(self, node);
}

void
jive_traversal_tracker_set_nodestate(jive_traversal_tracker * self, jive_node * node,
	jive_traversal_nodestate state)
{
	jive_tracker_set_nodestate(self, node, (size_t) state, 0);
}

jive_node *
jive_traversal_tracker_pop_top(jive_traversal_tracker * self)
{
	return jive_tracker_pop_top(self, (size_t) jive_traversal_nodestate_frontier);
}

jive_node *
jive_traversal_tracker_pop_bottom(jive_traversal_tracker * self)
{
	return jive_tracker_pop_bottom(self, (size_t) jive_traversal_nodestate_frontier);
}

void
jive_computation_tracker_init(jive_computation_tracker * self, jive_graph * graph)
{
	self->graph = graph;
	self->nodestates = jive_graph_reserve_tracker_depth_state(graph);
	self->slot = jive_graph_reserve_tracker_slot(graph);
}

void
jive_computation_tracker_fini(jive_computation_tracker * self)
{
	jive_graph_return_tracker_depth_state(self->graph, self->nodestates);
	jive_graph_return_tracker_slot(self->graph, self->slot);
}

static jive_tracker_nodestate *
jive_computation_tracker_map_node(jive_computation_tracker * self, jive_node * node)
{
	return jive_node_get_tracker_state(node, self->slot);
}

void
jive_computation_tracker_invalidate(jive_computation_tracker * self, jive_node * node)
{
	jive_tracker_nodestate * nodestate = jive_computation_tracker_map_node(self, node);
	if (nodestate->state == jive_tracker_nodestate_none) {
		jive_tracker_depth_state_add(self->nodestates, nodestate, node->depth_from_root);
		nodestate->state = 0;
	}
}

void
jive_computation_tracker_invalidate_below(jive_computation_tracker * self, jive_node * node)
{
	size_t n;
	for (n = 0; n < node->noutputs; n++) {
		jive::output * output = node->outputs[n];
		jive::input * user;
		JIVE_LIST_ITERATE(output->users, user, output_users_list) {
			jive_computation_tracker_invalidate(self, user->node());
		}
	}
}

jive_node *
jive_computation_tracker_pop_top(jive_computation_tracker * self)
{
	jive_tracker_nodestate * nodestate = jive_tracker_depth_state_pop_top(self->nodestates);
	if (nodestate)
		return nodestate->node;
	else
		return 0;
}
