/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_UTIL_SET_H
#define JIVE_UTIL_SET_H

#include <jive/context.h>
#include <jive/util/hash.h>

#define JIVE_DECLARE_SET_TYPE(set_type, item_type) \
struct set_type##_entry { \
	item_type  * item; \
	struct { \
		struct set_type##_entry * prev; \
		struct set_type##_entry * next; \
	} entry_chain; \
}; \
\
struct set_type##_bucket { \
	struct set_type##_entry * first; \
	struct set_type##_entry * last; \
}; \
\
typedef struct set_type set_type; \
struct set_type { \
	struct jive_context * context; \
	size_t nitems, nbuckets, mask; \
	struct set_type##_bucket * buckets; \
}; \
\
struct set_type##_iterator { \
	const struct set_type * set; \
	const struct set_type##_entry * entry; \
	size_t next_bucket; \
};

#define JIVE_DEFINE_SET_TYPE(set_type, item_type) \
\
static inline void \
set_type##_init(struct set_type * self, jive_context * context) \
{ \
	self->context = context; \
	self->nitems = self->nbuckets = self->mask = 0; \
	self->buckets = 0; \
} \
\
static inline void \
set_type##_fini(struct set_type * self) \
{ \
	size_t n; \
	for (n = 0; n < self->nbuckets; n++) { \
		struct set_type##_entry * entry, * next_entry; \
		JIVE_LIST_ITERATE_SAFE(self->buckets[n], entry, next_entry, entry_chain) { \
			JIVE_LIST_REMOVE(self->buckets[n], entry, entry_chain); \
			jive_context_free(self->context, entry); \
		} \
	} \
	jive_context_free(self->context, self->buckets); \
} \
\
static inline void \
set_type##_rehash(struct set_type * self) \
{ \
	size_t new_nbuckets = self->nbuckets * 2; \
	if (new_nbuckets == 0) new_nbuckets = 1; \
	struct set_type##_bucket * new_buckets; \
	new_buckets = jive_context_malloc(self->context, sizeof(*new_buckets) * new_nbuckets); \
	\
	size_t n; \
	for (n = 0; n < new_nbuckets; n++) \
		new_buckets[n].first = new_buckets[n].last = 0; \
	\
	for (n = 0; n < self->nbuckets; n++) { \
		while (self->buckets[n].first) { \
			struct set_type##_entry * entry = self->buckets[n].first; \
			JIVE_LIST_REMOVE(self->buckets[n], entry, entry_chain); \
			size_t hash = jive_ptr_hash(entry->item) & (new_nbuckets - 1); \
			JIVE_LIST_PUSH_BACK(new_buckets[hash], entry, entry_chain); \
		} \
	} \
	\
	jive_context_free(self->context, self->buckets); \
	self->buckets = new_buckets; \
	self->nbuckets = new_nbuckets; \
	self->mask = new_nbuckets - 1; \
} \
\
static inline bool \
set_type##_contains(const struct set_type * self, const item_type * item) \
{ \
	if (!self->nbuckets) \
		return false; \
	\
	size_t hash = jive_ptr_hash(item) & self->mask; \
	struct set_type##_entry * entry = self->buckets[hash].first; \
	while (entry) { \
		if (entry->item == item) \
			return true; \
		entry = entry->entry_chain.next; \
	} \
	\
	return false; \
} \
\
static inline void \
set_type##_insert(struct set_type * self, item_type * item) \
{ \
	if (set_type##_contains(self, item)) \
		return; \
	\
	self->nitems++; \
	if (self->nitems > self->nbuckets) \
		set_type##_rehash(self); \
	\
	struct set_type##_entry * entry = jive_context_malloc(self->context, sizeof(*entry)); \
	entry->item = item; \
	size_t hash = jive_ptr_hash(item) & self->mask; \
	JIVE_LIST_PUSH_BACK(self->buckets[hash], entry, entry_chain); \
} \
\
static inline void \
set_type##_remove(struct set_type * self, const item_type * item) \
{ \
	if (!self->nbuckets) \
		return; \
	\
	size_t hash = jive_ptr_hash(item) & self->mask; \
	struct set_type##_entry * entry = self->buckets[hash].first; \
	while (entry) { \
		if (entry->item == item) { \
			self->nitems--; \
			JIVE_LIST_REMOVE(self->buckets[hash], entry, entry_chain); \
			jive_context_free(self->context, entry); \
			break; \
		} \
		entry = entry->entry_chain.next; \
	} \
} \
\
static inline size_t \
set_type##_size(const struct set_type * self) \
{ \
	return self->nitems; \
} \
\
static inline void \
set_type##_clear(struct set_type * self) \
{ \
	size_t n; \
	for (n = 0; n < self->nbuckets; n++) { \
		struct set_type##_entry * entry, * next_entry; \
		JIVE_LIST_ITERATE_SAFE(self->buckets[n], entry, next_entry, entry_chain) { \
			JIVE_LIST_REMOVE(self->buckets[n], entry, entry_chain); \
			jive_context_free(self->context, entry); \
		} \
	} \
	self->nitems = self->nbuckets = self->mask = 0; \
} \
\
static inline void \
set_type##_iterator_next_bucket(struct set_type##_iterator * iterator) \
{ \
	while (!iterator->entry && iterator->next_bucket < iterator->set->nbuckets) { \
		iterator->entry = iterator->set->buckets[iterator->next_bucket].first; \
		iterator->next_bucket ++; \
	} \
} \
\
static inline void \
set_type##_iterator_next(struct set_type##_iterator * iterator) \
{ \
	iterator->entry = iterator->entry->entry_chain.next; \
	if (!iterator->entry) \
		set_type##_iterator_next_bucket(iterator); \
} \
\
static inline struct set_type##_iterator \
set_type##_begin(const struct set_type * self) \
{ \
	struct set_type##_iterator iterator; \
	iterator.set = self; \
	iterator.entry = 0; \
	iterator.next_bucket = 0; \
	set_type##_iterator_next_bucket(&iterator); \
	return iterator; \
} \
\
static inline bool \
set_type##_equals(const struct set_type * self, const struct set_type * other) \
{ \
	if (set_type##_size(self) != set_type##_size(other)) \
		return false; \
	\
	struct set_type##_iterator iterator; \
	for (iterator = set_type##_begin(self); iterator.entry; set_type##_iterator_next(&iterator)) \
		if (!set_type##_contains(other, iterator.entry->item)) \
			return false;	\
	\
	return true; \
} \
\
static inline void \
set_type##_copy(const struct set_type * self, struct set_type * other) \
{ \
	set_type##_clear(other); \
	struct set_type##_iterator iterator; \
	for (iterator = set_type##_begin(self); iterator.entry; set_type##_iterator_next(&iterator)) \
		set_type##_insert(other, iterator.entry->item); \
} \
\
static inline void \
set_type##_replace(struct set_type * self, const struct set_type * other) \
{ \
	set_type##_fini(self); \
	*self = *other; \
} \
\
set_type##_union(const struct set_type * self, const struct set_type * other, \
	struct set_type * result) \
{ \
	set_type##_clear(result); \
	struct set_type##_iterator iterator; \
	for (iterator = set_type##_begin(self); iterator.entry; set_type##_iterator_next(&iterator)) \
		set_type##_insert(result, iterator.entry->item); \
	for (iterator = set_type##_begin(other); iterator.entry; set_type##_iterator_next(&iterator)) \
		set_type##_insert(result, iterator.entry->item); \
} \
\
static inline void \
set_type##_intersection(const struct set_type * self, const struct set_type * other, \
	struct set_type * result) \
{ \
	set_type##_clear(result); \
	struct set_type##_iterator iterator; \
	for (iterator = set_type##_begin(self); iterator.entry; set_type##_iterator_next(&iterator)) {\
		if (set_type##_contains(other, iterator.entry->item)) \
			set_type##_insert(result, iterator.entry->item); \
	} \
} \

#define JIVE_SET_ITERATE(set_type, set, iterator) \
	for (iterator = set_type##_begin(&set); iterator.entry; set_type##_iterator_next(&iterator))

#endif
