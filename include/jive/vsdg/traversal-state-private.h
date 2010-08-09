#ifndef JIVE_VSDG_TRAVERSAL_STATE_PRIVATE_H
#define JIVE_VSDG_TRAVERSAL_STATE_PRIVATE_H

#include <jive/vsdg/traversal-state.h>
#include <jive/vsdg/traverser.h>
#include <jive/debug-private.h>
#include <jive/util/list.h>

void
_jive_traversal_state_init_slow(jive_traversal_state * self, jive_graph * graph);

static inline void
jive_traversal_state_init(jive_traversal_state * self, jive_graph * graph)
{
	self->graph = graph;
	
	size_t n;
	for(n = 0; n < graph->ntraverser_slots; n++) {
		if (!graph->traverser_slots[n].traverser) {
			self->index = n;
			self->cookie = graph->traverser_slots[n].cookie + 1;
			graph->traverser_slots[n].traverser = (void *) self; /* FIXME: correct type */
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
	self->graph->traverser_slots[self->index].traverser = 0;
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
