#ifndef JIVE_VSDG_TRAVERSER_H
#define JIVE_VSDG_TRAVERSER_H

#include <stdbool.h>
#include <stdlib.h>

#include <jive/vsdg/traversal-state.h>

typedef struct jive_traverser jive_traverser;
typedef struct jive_traverser_class jive_traverser_class;
typedef struct jive_traverser_graphstate jive_traverser_graphstate;

struct jive_traverser_graphstate {
	jive_traverser * traverser;
	size_t cookie;
};

struct jive_traverser {
	const jive_traverser_class * class_;
	
	jive_graph * graph;
	
	struct {
		struct jive_traversal_nodestate * first;
		struct jive_traversal_nodestate * last;
	} frontier;
};

struct jive_traverser_class {
	const jive_traverser_class * parent;
	
	void (*fini)(jive_traverser * self);
	
	struct jive_node * (*next)(jive_traverser * self);
};

void
jive_traverser_destroy(jive_traverser * self);

static inline struct jive_node *
jive_traverser_next(jive_traverser * self)
{
	return self->class_->next(self);
}

jive_traverser *
jive_topdown_traverser_create(struct jive_graph * graph);

jive_traverser *
jive_bottomup_traverser_create(struct jive_graph * graph);

bool
jive_traverser_node_is_unvisited(const jive_traverser * self, struct jive_node * node);

bool
jive_traverser_node_is_candidate(const jive_traverser * self, struct jive_node * node);

bool
jive_traverser_node_is_visited(const jive_traverser * self, struct jive_node * node);

#endif
