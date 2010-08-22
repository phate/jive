#ifndef JIVE_VSDG_TRAVERSER_PRIVATE_H
#define JIVE_VSDG_TRAVERSER_PRIVATE_H

#include <jive/vsdg/traverser.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/graph.h>
#include <jive/internal/compiler.h>
#include <jive/debug-private.h>
#include <jive/util/list.h>

struct jive_traversal_state {
	struct jive_graph * graph;
	size_t cookie;
	size_t index;
};

extern const jive_traverser_class JIVE_TRAVERSER;

void
_jive_traverser_fini(jive_traverser * self);

void
_jive_traverser_init(jive_traverser * self, jive_graph * graph);

void
jive_traverser_add_frontier(jive_traverser * self, jive_traversal_state * state_tracker, struct jive_traversal_nodestate * nodestate);

extern const jive_traverser_class JIVE_FULL_TRAVERSER;

typedef struct jive_full_traverser jive_full_traverser;

struct jive_full_traverser {
	jive_traverser base;
	
	struct jive_notifier
		* node_create,
		* node_destroy,
		* input_change;
	
	jive_traversal_state state_tracker;
};

void
_jive_full_traverser_fini(jive_traverser * self_);

void
_jive_full_traverser_init(jive_full_traverser * self, jive_graph * graph);

void
jive_full_traverser_add_frontier(jive_full_traverser * self,  struct jive_traversal_nodestate * nodestate);

/* utilities to track traversal state */

typedef struct jive_traverser_graphstate jive_traverser_graphstate;

struct jive_traverser_graphstate {
	jive_traversal_state * traversal_state;
	size_t cookie;
};

void
_jive_traversal_state_init_slow(jive_traversal_state * self, jive_graph * graph);

static inline void
jive_traversal_state_init(jive_traversal_state * self, jive_graph * graph)
{
	self->graph = graph;
	
	size_t n;
	for(n = 0; n < graph->ntraverser_slots; n++) {
		if (!graph->traverser_slots[n].traversal_state) {
			self->index = n;
			self->cookie = graph->traverser_slots[n].cookie + 1;
			graph->traverser_slots[n].traversal_state = self;
			return;
		}
	}
	
	_jive_traversal_state_init_slow(self, graph);
}

static inline void
jive_traversal_state_fini(jive_traversal_state * self)
{
	self->graph->traverser_slots[self->index].cookie = self->cookie + 1;
	/* TODO: reset cookies iff == 0 ? or don't reuse this slot? */
	DEBUG_ASSERT(self->graph->traverser_slots[self->index].cookie);
	self->graph->traverser_slots[self->index].traversal_state = 0;
}

jive_traversal_nodestate *
jive_traversal_state_alloc_nodestate(const jive_traversal_state * self, jive_node * node);

static inline jive_traversal_nodestate *
jive_traversal_state_get_nodestate(const jive_traversal_state * self, jive_node * node)
{
	if (likely(self->index < node->ntraverser_slots)) {
		jive_traversal_nodestate * nodestate = node->traverser_slots[self->index];
		if (likely(nodestate != 0)) return nodestate;
	}
	return jive_traversal_state_alloc_nodestate(self, node);
}

static inline void
_jive_traversal_nodestate_unlink(const jive_traversal_state * self, jive_traversal_nodestate * nodestate)
{
	if (nodestate->traverser && nodestate->cookie == self->cookie)
		JIVE_LIST_REMOVE(nodestate->traverser->frontier, nodestate, traverser_node_list);
}

static inline void
jive_traversal_state_mark_ahead(const jive_traversal_state * self, jive_traversal_nodestate * nodestate)
{
	_jive_traversal_nodestate_unlink(self, nodestate);
	nodestate->cookie = self->cookie - 1;
}

static inline void
jive_traversal_state_mark_frontier(const jive_traversal_state * self, jive_traversal_nodestate * nodestate)
{
	_jive_traversal_nodestate_unlink(self, nodestate);
	nodestate->cookie = self->cookie;
}

static inline void
jive_traversal_state_mark_behind(const jive_traversal_state * self, jive_traversal_nodestate * nodestate)
{
	_jive_traversal_nodestate_unlink(self, nodestate);
	nodestate->cookie = self->cookie + 1;
}

static inline bool
jive_traversal_state_is_ahead(const jive_traversal_state * self, jive_traversal_nodestate * nodestate)
{
	return nodestate->cookie < self->cookie;
}

static inline bool
jive_traversal_state_is_frontier(const jive_traversal_state * self, jive_traversal_nodestate * nodestate)
{
	return nodestate->cookie == self->cookie;
}

static inline bool
jive_traversal_state_is_behind(const jive_traversal_state * self, jive_traversal_nodestate * nodestate)
{
	return nodestate->cookie > self->cookie;
}

static inline void
jive_traversal_state_forget(const jive_traversal_state * self, jive_traversal_nodestate * nodestate)
{
	_jive_traversal_nodestate_unlink(self, nodestate);
}


#endif
