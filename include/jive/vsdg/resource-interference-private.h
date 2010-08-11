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

JIVE_DEFINE_HASH_TYPE(jive_resource_interference_hash, struct jive_resource_interference_part, struct jive_resource *, resource, chain);

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

#endif
