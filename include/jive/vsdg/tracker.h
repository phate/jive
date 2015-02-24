/*
 * Copyright 2010 2011 2012 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_TRACKER_H
#define JIVE_VSDG_TRACKER_H

#include <stdbool.h>
#include <stddef.h>

#include <jive/util/callbacks.h>

struct jive_graph;
struct jive_node;
struct jive_notifier;
struct jive_region;
struct jive_tracker_depth_state;

static const size_t jive_tracker_nodestate_none = (size_t) -1;

struct jive_tracker_slot {
	size_t index;
	size_t cookie;
};

/* Track states of nodes within the graph. Each node can logically be in
 * one of the numbered states, plus another "initial" state. All nodes are
 * at the beginning assumed to be implicitly in this "initial" state. */
struct jive_tracker {
public:
	jive_tracker(jive_graph * graph, size_t nstates);
	~jive_tracker();

	jive_graph * graph_;
	/* FIXME: need RAII idiom for slot reservation */
	jive_tracker_slot slot_;
	/* FIXME: need RAII idiom for state reservation */
	std::vector<jive_tracker_depth_state *> states_;

	jive::callback depth_callback_, destroy_callback_;
};

int
jive_tracker_get_nodestate(jive_tracker * self, struct jive_node * node);

int
jive_tracker_get_nodetag(jive_tracker * self, struct jive_node * node);

void
jive_tracker_set_nodestate(jive_tracker * self, struct jive_node * node, size_t state, int tag);

struct jive_node *
jive_tracker_pop_top(jive_tracker * self, size_t state);

struct jive_node *
jive_tracker_pop_bottom(jive_tracker * self, size_t state);

typedef enum jive_traversal_nodestate {
	jive_traversal_nodestate_ahead = -1,
	jive_traversal_nodestate_frontier = 0,
	jive_traversal_nodestate_behind = +1
} jive_traversal_nodestate;

class jive_traversal_tracker final : public jive_tracker {
public:
	jive_traversal_tracker(jive_graph * graph);
};

jive_traversal_nodestate
jive_traversal_tracker_get_nodestate(jive_traversal_tracker * self, struct jive_node * node);

void
jive_traversal_tracker_set_nodestate(jive_traversal_tracker * self, struct jive_node * node,
	jive_traversal_nodestate state);

struct jive_node *
jive_traversal_tracker_pop_top(jive_traversal_tracker * self);

struct jive_node *
jive_traversal_tracker_pop_bottom(jive_traversal_tracker * self);

struct jive_computation_tracker {
	struct jive_tracker_depth_state * nodestates;
	struct jive_graph * graph;
	jive_tracker_slot slot;
};

void
jive_computation_tracker_init(jive_computation_tracker * self, struct jive_graph * graph);

void
jive_computation_tracker_fini(jive_computation_tracker * self);

void
jive_computation_tracker_invalidate(jive_computation_tracker * self, struct jive_node * node);

void
jive_computation_tracker_invalidate_below(jive_computation_tracker * self, struct jive_node * node);

struct jive_node *
jive_computation_tracker_pop_top(jive_computation_tracker * self);

#endif
