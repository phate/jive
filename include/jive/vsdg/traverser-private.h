#ifndef JIVE_VSDG_TRAVERSER_PRIVATE_H
#define JIVE_VSDG_TRAVERSER_PRIVATE_H

#include <jive/vsdg/traverser.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/graph.h>
#include <jive/internal/compiler.h>

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

#endif
