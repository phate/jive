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

static inline void
jive_resource_interference_add(jive_resource * first, jive_resource * second)
{
	jive_resource_interference * i;
	jive_resource_interference_part * part = jive_resource_interference_hash_lookup(&first->interference, second);
	if (part) i = part->whole;
	else i = jive_resource_interference_create(first, second);
	i->count ++;
}

static inline void
jive_resource_interference_remove(jive_resource * first, jive_resource * second)
{
	jive_resource_interference * i;
	jive_resource_interference_part * part = jive_resource_interference_hash_lookup(&first->interference, second);
	i = part->whole;
	i->count --;
	if (!i->count) jive_resource_interference_destroy(i);
}

/* TODO: iterator support */

#endif
