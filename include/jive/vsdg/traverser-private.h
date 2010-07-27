#ifndef JIVE_VSDG_TRAVERSER_PRIVATE_H
#define JIVE_VSDG_TRAVERSER_PRIVATE_H

#include <jive/vsdg/traverser.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/graph.h>
#include <jive/internal/compiler.h>

typedef struct jive_traverser_nodestate jive_traverser_nodestate;
typedef struct jive_traverser_graphstate jive_traverser_graphstate;

struct jive_traverser_graphstate {
	jive_traverser * traverser;
	size_t cookie;
};

struct jive_traverser_nodestate {
	jive_node * node;
	size_t cookie;
	struct {
		jive_traverser_nodestate * prev;
		jive_traverser_nodestate * next;
	} traverser_node_list;
};

void
_jive_traverser_fini(jive_traverser * self);

void
_jive_traverser_init(jive_traverser * self, jive_graph * graph);

jive_traverser_nodestate *
jive_traverser_alloc_nodestate(const jive_traverser * self, jive_node * node);

static inline jive_traverser_nodestate *
jive_traverser_get_nodestate(const jive_traverser * self, jive_node * node)
{
	if (likely(self->index < node->ntraverser_slots)) {
		jive_traverser_nodestate * nodestate = node->traverser_slots[self->index];
		if (likely(nodestate != 0)) return nodestate;
	}
	return jive_traverser_alloc_nodestate(self, node);
}

bool
jive_traverser_node_is_unvisited(const jive_traverser * self, jive_node * node);

bool
jive_traverser_node_is_candidate(const jive_traverser * self, jive_node * node);

bool
jive_traverser_node_is_visited(const jive_traverser * self, jive_node * node);

#endif
