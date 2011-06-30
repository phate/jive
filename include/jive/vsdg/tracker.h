#ifndef JIVE_VSDG_TRACKER_H
#define JIVE_VSDG_TRACKER_H

#include <stdbool.h>
#include <stddef.h>

/*#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>*/

struct jive_context;
struct jive_graph;
struct jive_node;
struct jive_notifier;
struct jive_region;

typedef struct jive_tracker jive_tracker;
typedef struct jive_tracker_slot jive_tracker_slot;
typedef struct jive_traversal_tracker jive_traversal_tracker;
typedef struct jive_computation_tracker jive_computation_tracker;

static const size_t jive_tracker_nodestate_none = (size_t) -1;

struct jive_tracker_slot {
	size_t index;
	size_t cookie;
};

struct jive_tracker {
	struct jive_graph * graph;
	struct jive_tracker_depth_state ** states;
	size_t nstates;
	struct jive_notifier * callbacks[2];
	jive_tracker_slot slot;
};

void
jive_tracker_init(jive_tracker * self, struct jive_graph * graph, size_t nstates);

void
jive_tracker_fini(jive_tracker * self);

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

typedef enum {
	jive_traversal_nodestate_ahead = -1,
	jive_traversal_nodestate_frontier = 0,
	jive_traversal_nodestate_behind = +1
} jive_traversal_nodestate;

struct jive_traversal_tracker {
	jive_tracker base;
};

void
jive_traversal_tracker_init(jive_traversal_tracker * self, struct jive_graph * graph);

void
jive_traversal_tracker_fini(jive_traversal_tracker * self);

jive_traversal_nodestate
jive_traversal_tracker_get_nodestate(jive_traversal_tracker * self, struct jive_node * node);

void
jive_traversal_tracker_set_nodestate(jive_traversal_tracker * self, struct jive_node * node, jive_traversal_nodestate state);

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
