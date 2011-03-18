#ifndef JIVE_VSDG_RESOURCE_PRIVATE_H
#define JIVE_VSDG_RESOURCE_PRIVATE_H

#include <jive/vsdg/resource.h>

typedef struct jive_resource_class_count_iterator jive_resource_class_count_iterator;

void
jive_resource_class_count_clear(jive_resource_class_count * self, jive_context * context);

static inline void
jive_resource_class_count_init(jive_resource_class_count * self)
{
	self->nitems = self->nbuckets = 0;
	self->buckets = 0;
}

static inline void
jive_resource_class_count_fini(jive_resource_class_count * self, jive_context * context)
{
	if (self->buckets) {
		jive_resource_class_count_clear(self, context);
		jive_context_free(context, self->buckets);
	}
}

const struct jive_resource_class *
jive_resource_class_count_add(jive_resource_class_count * self, jive_context * context, const struct jive_resource_class * resource_class);

void
jive_resource_class_count_max(jive_resource_class_count * self, jive_context * context, const struct jive_resource_class * resource_class, size_t count);

void
jive_resource_class_count_sub(jive_resource_class_count * self, jive_context * context, const struct jive_resource_class * resource_class);

const struct jive_resource_class *
jive_resource_class_count_change(jive_resource_class_count * self, jive_context * context, const struct jive_resource_class * old_resource_class, const struct jive_resource_class * new_resource_class);

const struct jive_resource_class *
jive_resource_class_count_check_add(const jive_resource_class_count * self, const struct jive_resource_class * resource_class);

const struct jive_resource_class *
jive_resource_class_count_check_change(const jive_resource_class_count * self, const struct jive_resource_class * old_resource_class, const struct jive_resource_class * new_resource_class);

void
jive_resource_class_count_copy(jive_resource_class_count * self, jive_context * context, const jive_resource_class_count * src);

/* iterators */
struct jive_resource_class_count_iterator {
	jive_resource_class_count_item * entry;
	size_t next_bucket;
	jive_resource_class_count * count;
};

static inline void
jive_resource_class_count_iterator_next_bucket(jive_resource_class_count_iterator * i)
{
	while((i->next_bucket < i->count->nbuckets) && (!i->entry)) {
		i->entry = i->count->buckets[i->next_bucket].first;
		i->next_bucket ++;
	}
}

static inline jive_resource_class_count_iterator
jive_resource_class_count_begin(jive_resource_class_count * self)
{
	jive_resource_class_count_iterator i;
	i.count = self;
	i.next_bucket = 0;
	i.entry = 0;
	jive_resource_class_count_iterator_next_bucket(&i);
	return i;
}

static inline void
jive_resource_class_count_iterator_next(jive_resource_class_count_iterator * i)
{
	i->entry = i->entry->chain.next;
	jive_resource_class_count_iterator_next_bucket(i);
}

#endif
