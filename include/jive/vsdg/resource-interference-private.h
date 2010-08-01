#ifndef JIVE_RESOURCE_INTERFERENCE_PRIVATE_H
#define JIVE_RESOURCE_INTERFERENCE_PRIVATE_H

#include <stdlib.h>

#include <jive/vsdg/resource-interference.h>
#include <jive/context.h>
#include <jive/vsdg/basetype.h>

struct jive_resource_interference_part {
	jive_resource * resource;
	struct {
		jive_resource_interference_part * prev;
		jive_resource_interference_part * next;
	} chain;
	jive_resource_interference * whole;
};

struct jive_resource_interference {
	jive_resource_interference_part first;
	jive_resource_interference_part second;
	size_t count;
};

static inline void
jive_resource_interference_hash_init(jive_resource_interference_hash * self)
{
	self->nitems = self->nbuckets = 0;
	self->buckets = 0;
}

static inline void
jive_resource_interference_hash_fini(jive_resource_interference_hash * self, struct jive_context * context)
{
	if (self->buckets) jive_context_free(context, self->buckets);
}

static inline jive_resource_interference_part *
jive_resource_interference_hash_lookup(const jive_resource_interference_hash * self, const struct jive_resource * resource)
{
	if (!self->nbuckets) return 0;
	size_t hash = ((size_t) resource) % self->nbuckets;
	jive_resource_interference_part * i = self->buckets[hash].first;
	while(i && i->resource != resource) i = i->chain.next;
	return i;
}

jive_resource_interference *
jive_resource_interference_create(struct jive_resource * first, struct jive_resource * second);

void
jive_resource_interference_destroy(jive_resource_interference * self);

static inline size_t
jive_resource_interference_add(jive_resource * first, jive_resource * second)
{
	jive_resource_interference * i;
	jive_resource_interference_part * part = jive_resource_interference_hash_lookup(&first->interference, second);
	if (part) i = part->whole;
	else i = jive_resource_interference_create(first, second);
	return i->count ++;
}

static inline size_t
jive_resource_interference_remove(jive_resource * first, jive_resource * second)
{
	jive_resource_interference * i;
	jive_resource_interference_part * part = jive_resource_interference_hash_lookup(&first->interference, second);
	i = part->whole;
	size_t count = -- (i->count);
	if (!i->count) jive_resource_interference_destroy(i);
	return count;
}

typedef struct jive_resource_interference_iterator jive_resource_interference_iterator;
struct jive_resource_interference_iterator {
	const jive_resource_interference_hash * container;
	jive_resource_interference_part * pos;
	size_t next_bucket;
};

static inline void
jive_resource_interference_iterator_next_bucket(jive_resource_interference_iterator * i)
{
	while(i->next_bucket < i->container->nbuckets && !i->pos) {
		i->pos = i->container->buckets[i->next_bucket].first;
		i->next_bucket ++;
	}
}

static inline void
jive_resource_interference_iterator_next(jive_resource_interference_iterator * i)
{
	i->pos = i->pos->chain.next;
	if (!i->pos) jive_resource_interference_iterator_next_bucket(i);
}

static inline jive_resource_interference_iterator
jive_resource_interference_begin(const jive_resource_interference_hash * self)
{
	jive_resource_interference_iterator i;
	i.container = self;
	i.next_bucket = 0;
	i.pos = 0;
	jive_resource_interference_iterator_next_bucket(&i);
	return i;
}

#define JIVE_RESOURCE_INTERFERENCE_ITERATE(container, iterator) \
	for(iterator = jive_resource_interference_begin(&(container)); iterator.pos; jive_resource_interference_iterator_next(&iterator))

#endif
