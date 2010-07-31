#ifndef JIVE_VSDG_CROSSINGS_PRIVATE_H
#define JIVE_VSDG_CROSSINGS_PRIVATE_H

#include <jive/context.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/basetype.h>
#include <jive/debug-private.h>

typedef struct jive_node_resource_interaction jive_node_resource_interaction;
typedef struct jive_node_interaction_iterator jive_node_interaction_iterator;
typedef struct jive_resource_interaction_iterator jive_resource_interaction_iterator;

struct jive_node_resource_interaction {
	struct jive_node * node;
	struct jive_resource * resource;
	size_t before_count;
	size_t crossed_count;
	size_t after_count;
	
	struct {
		jive_node_resource_interaction * prev;
		jive_node_resource_interaction * next;
	} same_node_list;
	
	struct {
		jive_node_resource_interaction * prev;
		jive_node_resource_interaction * next;
	} same_resource_list;
};

void
jive_node_resource_interaction_destroy(jive_node_resource_interaction * self);

static inline jive_node_resource_interaction *
jive_node_resource_interaction_lookup(const jive_node * node, const jive_resource * resource)
{
	if (!node->resource_interaction.nbuckets) return 0;
	
	size_t hash_by_resource = ((size_t)resource) % node->resource_interaction.nbuckets;
	
	jive_node_resource_interaction * p = node->resource_interaction.buckets[hash_by_resource].first;
	while(p && p->resource != resource) p = p->same_node_list.next;
	return p;
}

jive_node_resource_interaction *
jive_node_resource_interaction_create_slow(jive_node * node, jive_resource * resource);

static inline jive_node_resource_interaction *
jive_node_resource_interaction_create(jive_node * node, jive_resource * resource)
{
	jive_node_resource_interaction * i;
	i = jive_node_resource_interaction_lookup(node, resource);
	if (i) return i;
	
	return jive_node_resource_interaction_create_slow(node, resource);
}

/* test whether all of the counst have dropped to zero
and possibly discards this interaction point */
static inline void
jive_node_resource_interaction_check_discard(jive_node_resource_interaction * self)
{
	if (self->before_count | self->crossed_count | self->after_count) return;
	jive_node_resource_interaction_destroy(self);
}

/* interaction points of node, hashed by resource */

static inline void
jive_resource_interaction_init(jive_resource_interaction * self)
{
	self->nbuckets = self->nitems = 0;
	self->buckets = 0;
}

static inline void
jive_resource_interaction_fini(jive_resource_interaction * self, jive_context * context)
{
	if (self->buckets) jive_context_free(context, self->buckets);
}

struct jive_resource_interaction_iterator {
	const jive_resource_interaction * container;
	jive_node_resource_interaction * pos;
	size_t next_bucket;
};

static inline void
jive_resource_interaction_iterator_next_bucket(jive_resource_interaction_iterator * i)
{
	while(i->next_bucket < i->container->nbuckets && !i->pos) {
		i->pos = i->container->buckets[i->next_bucket].first;
		i->next_bucket ++;
	}
}

static inline void
jive_resource_interaction_iterator_next(jive_resource_interaction_iterator * i)
{
	i->pos = i->pos->same_node_list.next;
	if (!i->pos) jive_resource_interaction_iterator_next_bucket(i);
}

static inline jive_resource_interaction_iterator
jive_resource_interaction_begin(const jive_resource_interaction * self)
{
	jive_resource_interaction_iterator i;
	i.container = self;
	i.next_bucket = 0;
	i.pos = 0;
	jive_resource_interaction_iterator_next_bucket(&i);
	return i;
}

#define JIVE_RESOURCE_INTERACTION_ITERATE(container, iterator) \
	for(iterator = jive_resource_interaction_begin(&(container)); iterator.pos; jive_resource_interaction_iterator_next(&iterator))

/* interaction points of single node, hashed by resource */

static inline void
jive_node_interaction_init(jive_node_interaction * self)
{
	self->first = self->last = 0;
}

struct jive_node_interaction_iterator {
	jive_node_resource_interaction * pos;
};

static inline void
jive_node_interaction_iterator_next(jive_node_interaction_iterator * i)
{
	i->pos = i->pos->same_resource_list.next;
}

static inline jive_node_interaction_iterator
jive_node_interaction_begin(const jive_node_interaction * self)
{
	jive_node_interaction_iterator i;
	i.pos = self->first;
	return i;
}

#define JIVE_NODE_INTERACTION_ITERATE(container, iterator) \
	for(iterator = jive_node_interaction_begin(); iterator.pos; jive_node_interaction_iterator_next)

#endif
