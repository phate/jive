#ifndef JIVE_VSDG_TRAVERSER_H
#define JIVE_VSDG_TRAVERSER_H

#include <stdbool.h>
#include <stdlib.h>

struct jive_graph;
struct jive_node;

typedef struct jive_traverser jive_traverser;
typedef struct jive_traverser_class jive_traverser_class;
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

struct jive_traverser {
	const jive_traverser_class * class_;
	
	struct jive_graph * graph;
	
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

#endif
