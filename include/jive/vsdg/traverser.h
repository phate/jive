#ifndef JIVE_VSDG_TRAVERSER_H
#define JIVE_VSDG_TRAVERSER_H

#include <stdbool.h>
#include <stdlib.h>

typedef struct jive_traverser jive_traverser;
typedef struct jive_traverser_class jive_traverser_class;

struct jive_traverser_nodestate;

struct jive_traverser {
	const jive_traverser_class * class_;
	
	struct jive_graph * graph;
	
	struct {
		struct jive_traverser_nodestate * first;
		struct jive_traverser_nodestate * last;
	} next_nodes;
	
	struct {
		struct jive_traverser_nodestate * first;
		struct jive_traverser_nodestate * last;
	} visited_nodes;
	
	struct jive_notifier
		* node_create,
		* node_destroy,
		* input_change;
	
	size_t index, cookie;
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
