#ifndef JIVE_VSDG_TRAVERSAL_STATE_H
#define JIVE_VSDG_TRAVERSAL_STATE_H

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/internal/compiler.h>

struct jive_traverser;

typedef struct jive_traversal_nodestate jive_traversal_nodestate;
typedef struct jive_traversal_state jive_traversal_state;

struct jive_traversal_nodestate {
	struct jive_node * node;
	struct jive_traverser * traverser;
	size_t cookie;
	struct {
		jive_traversal_nodestate * prev;
		jive_traversal_nodestate * next;
	} traverser_node_list;
};

struct jive_traversal_state {
	struct jive_graph * graph;
	size_t cookie;
	size_t index;
};

#endif
