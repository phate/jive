/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_RESOURCE_PRIVATE_H
#define JIVE_VSDG_RESOURCE_PRIVATE_H

#include <jive/vsdg/resource.h>

typedef struct jive_resource_class_count_iterator jive_resource_class_count_iterator;

void
jive_resource_class_count_clear(jive_resource_class_count * self);

static inline void
jive_resource_class_count_init(jive_resource_class_count * self)
{
	self->nitems = self->nbuckets = self->mask = 0;
	self->items.first = self->items.last = 0;
}

static inline void
jive_resource_class_count_fini(jive_resource_class_count * self)
{
	if (!self->buckets.empty())
		jive_resource_class_count_clear(self);
}

const struct jive_resource_class *
jive_resource_class_count_add(jive_resource_class_count * self,
	const struct jive_resource_class * resource_class);

void
jive_resource_class_count_max(jive_resource_class_count * self,
	const struct jive_resource_class * resource_class, size_t count);

void
jive_resource_class_count_sub(jive_resource_class_count * self,
	const struct jive_resource_class * resource_class);

size_t
jive_resource_class_count_get(const jive_resource_class_count * self,
	const struct jive_resource_class * resource_class);

const struct jive_resource_class *
jive_resource_class_count_change(jive_resource_class_count * self,
	const struct jive_resource_class * old_resource_class,
	const struct jive_resource_class * new_resource_class);

const struct jive_resource_class *
jive_resource_class_count_check_add(const jive_resource_class_count * self,
	const struct jive_resource_class * resource_class);

const struct jive_resource_class *
jive_resource_class_count_check_change(const jive_resource_class_count * self,
	const struct jive_resource_class * old_resource_class,
	const struct jive_resource_class * new_resource_class);

void
jive_resource_class_count_copy(jive_resource_class_count * self,
	const jive_resource_class_count * src);

bool
jive_resource_class_count_equals(const jive_resource_class_count * self,
	const jive_resource_class_count * other);

void
jive_resource_class_count_update_union(jive_resource_class_count * self,
	const jive_resource_class_count * other);

void
jive_resource_class_count_update_intersection(jive_resource_class_count * self,
	const jive_resource_class_count * other);

void
jive_resource_class_count_update_add(jive_resource_class_count * self,
	const jive_resource_class_count * other);

/* iterators */
struct jive_resource_class_count_iterator {
	jive_resource_class_count_item * entry;
};

static inline jive_resource_class_count_iterator
jive_resource_class_count_begin(const jive_resource_class_count * self)
{
	jive_resource_class_count_iterator i;
	i.entry = self->items.first;
	return i;
}

static inline void
jive_resource_class_count_iterator_next(jive_resource_class_count_iterator * i)
{
	i->entry = i->entry->item_list.next;
}

#endif
