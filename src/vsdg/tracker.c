/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/tracker.h>

#include <jive/common.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/graph-private.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/tracker-private.h>

static jive_tracker_nodestate *
jive_tracker_map_node(jive_tracker * self, jive_node * node)
{
	return jive_node_get_tracker_state(node, self->slot);
}

static void
node_depth_change(void * closure, jive_node * node, size_t old_depth)
{
	jive_tracker * self = (jive_tracker *) closure;
	jive_tracker_nodestate * nodestate = jive_tracker_map_node(self, node);
	if (nodestate->state >= self->nstates)
		return;
	jive_tracker_depth_state_remove(self->states[nodestate->state], nodestate, old_depth);
	jive_tracker_depth_state_add(self->states[nodestate->state], nodestate, node->depth_from_root);
	
}

static void
node_destroy(void * closure, jive_node * node)
{
	jive_tracker * self = (jive_tracker *) closure;
	jive_tracker_nodestate * nodestate = jive_tracker_map_node(self, node);
	if (nodestate->state >= self->nstates)
		return;
	jive_tracker_depth_state_remove(self->states[nodestate->state], nodestate, node->depth_from_root);
}

void
jive_tracker_init(jive_tracker * self, jive_graph * graph, size_t nstates)
{
	self->graph = graph;
	self->states = new jive_tracker_depth_state*[nstates];
	size_t n;
	for (n = 0; n < nstates; n++)
		self->states[n] = jive_graph_reserve_tracker_depth_state(graph);
	self->nstates = nstates;
	
	self->slot = jive_graph_reserve_tracker_slot(graph);
	
	self->callbacks[0] = jive_node_depth_notifier_slot_connect(&graph->on_node_depth_change,
		node_depth_change, self);
	self->callbacks[1] = jive_node_notifier_slot_connect(&graph->on_node_destroy, node_destroy, self);
}

void
jive_tracker_fini(jive_tracker * self)
{
	size_t n;
	for (n = 0; n < self->nstates; n++)
		jive_graph_return_tracker_depth_state(self->graph, self->states[n]);
	delete[] self->states;
	jive_notifier_disconnect(self->callbacks[0]);
	jive_notifier_disconnect(self->callbacks[1]);
	jive_graph_return_tracker_slot(self->graph, self->slot);
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
		if (nodestate->state < self->nstates) {
			jive_tracker_depth_state_remove(self->states[nodestate->state], nodestate,
				node->depth_from_root);
		}
		nodestate->state = state;
		if (nodestate->state < self->nstates) {
			jive_tracker_depth_state_add(self->states[nodestate->state], nodestate,
				node->depth_from_root);
		}
	}
}

jive_node *
jive_tracker_pop_top(jive_tracker * self, size_t state)
{
	jive_tracker_nodestate * nodestate = jive_tracker_depth_state_pop_top(self->states[state]);
	if (nodestate)
		return nodestate->node;
	else
		return 0;
}

jive_node *
jive_tracker_pop_bottom(jive_tracker * self, size_t state)
{
	jive_tracker_nodestate * nodestate = jive_tracker_depth_state_pop_bottom(self->states[state]);
	if (nodestate)
		return nodestate->node;
	else
		return 0;
}

void
jive_traversal_tracker_init(jive_traversal_tracker * self, jive_graph * graph)
{
	jive_tracker_init(&self->base, graph, 2);
}

void
jive_traversal_tracker_fini(jive_traversal_tracker * self)
{
	jive_tracker_fini(&self->base);
}

jive_traversal_nodestate
jive_traversal_tracker_get_nodestate(jive_traversal_tracker * self, jive_node * node)
{
	return (jive_traversal_nodestate) jive_tracker_get_nodestate(&self->base, node);
}

void
jive_traversal_tracker_set_nodestate(jive_traversal_tracker * self, jive_node * node,
	jive_traversal_nodestate state)
{
	jive_tracker_set_nodestate(&self->base, node, (size_t) state, 0);
}

jive_node *
jive_traversal_tracker_pop_top(jive_traversal_tracker * self)
{
	return jive_tracker_pop_top(&self->base, (size_t) jive_traversal_nodestate_frontier);
}

jive_node *
jive_traversal_tracker_pop_bottom(jive_traversal_tracker * self)
{
	return jive_tracker_pop_bottom(&self->base, (size_t) jive_traversal_nodestate_frontier);
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
			jive_computation_tracker_invalidate(self, user->node);
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
