#ifndef JIVE_REGCLS_COUNT_PRIVATE_H
#define JIVE_REGCLS_COUNT_PRIVATE_H

#include <jive/vsdg/regcls-count.h>

typedef struct jive_regcls_count_iterator jive_regcls_count_iterator;

void
jive_regcls_count_clear(jive_regcls_count * self, jive_context * context);

static inline void
jive_regcls_count_init(jive_regcls_count * self)
{
	self->nitems = self->nbuckets = 0;
	self->buckets = 0;
}

static inline void
jive_regcls_count_fini(jive_regcls_count * self, jive_context * context)
{
	if (self->buckets) {
		jive_regcls_count_clear(self, context);
		jive_context_free(context, self->buckets);
	}
}

const struct jive_regcls *
jive_regcls_count_add(jive_regcls_count * self, jive_context * context, const struct jive_regcls * regcls);

void
jive_regcls_count_max(jive_regcls_count * self, jive_context * context, const struct jive_regcls * regcls, size_t count);

void
jive_regcls_count_sub(jive_regcls_count * self, jive_context * context, const struct jive_regcls * regcls);

const struct jive_regcls *
jive_regcls_count_change(jive_regcls_count * self, jive_context * context, const struct jive_regcls * old_regcls, const struct jive_regcls * new_regcls);

const struct jive_regcls *
jive_regcls_count_check_add(const jive_regcls_count * self, const struct jive_regcls * regcls);

const struct jive_regcls *
jive_regcls_count_check_change(const jive_regcls_count * self, const struct jive_regcls * old_regcls, const struct jive_regcls * new_regcls);

void
jive_regcls_count_copy(jive_regcls_count * self, jive_context * context, const jive_regcls_count * src);

/* iterators */
struct jive_regcls_count_iterator {
	jive_regcls_count_item * entry;
	size_t next_bucket;
	jive_regcls_count * count;
};

static inline void
jive_regcls_count_iterator_next_bucket(jive_regcls_count_iterator * i)
{
	while((i->next_bucket < i->count->nbuckets) && (!i->entry)) {
		i->entry = i->count->buckets[i->next_bucket].first;
		i->next_bucket ++;
	}
}

static inline jive_regcls_count_iterator
jive_regcls_count_begin(jive_regcls_count * self)
{
	jive_regcls_count_iterator i;
	i.count = self;
	i.next_bucket = 0;
	i.entry = 0;
	jive_regcls_count_iterator_next_bucket(&i);
	return i;
}

static inline void
jive_regcls_count_iterator_next(jive_regcls_count_iterator * i)
{
	i->entry = i->entry->chain.next;
	jive_regcls_count_iterator_next_bucket(i);
}

#endif
