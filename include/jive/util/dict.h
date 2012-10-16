/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_UTIL_DICT_H
#define JIVE_UTIL_DICT_H

#include <stddef.h>
#include <string.h>

#include <jive/util/list.h>

static inline size_t
jive_str_hash(const char * ptr)
{
	size_t hash = 0;
	while (*ptr) {
		hash = hash * 5 + *ptr;
		++ ptr;
	}
	return hash;
}

#define JIVE_DECLARE_DICT_TYPE(dict_type, entry_type, key_member, chain_member) \
struct dict_type##_bucket { \
	entry_type * first; \
	entry_type * last; \
}; \
 \
struct dict_type { \
	struct jive_context * context; \
	size_t nitems, nbuckets, mask; \
	struct dict_type##_bucket * buckets; \
}; \
 \
struct dict_type##_iterator { \
	const struct dict_type * hash; \
	entry_type * entry; \
	size_t next_bucket; \
};

#define JIVE_DEFINE_DICT_TYPE(dict_type, entry_type, key_member, chain_member) \
 \
static inline void \
dict_type##_init(struct dict_type * self, jive_context * context) \
{ \
	self->context = context; \
	self->nitems = self->nbuckets = self->mask = 0; \
	self->buckets = 0; \
} \
 \
static inline void \
dict_type##_fini(struct dict_type * self) \
{ \
	jive_context_free(self->context, self->buckets); \
} \
 \
static inline void \
dict_type##_rehash(struct dict_type * self) \
{ \
	size_t new_nbuckets = self->nbuckets * 2; \
	if (new_nbuckets == 0) new_nbuckets = 1; \
	struct dict_type##_bucket * new_buckets; \
	new_buckets = jive_context_malloc(self->context, sizeof(*new_buckets) * new_nbuckets); \
	 \
	size_t n; \
	for(n=0; n<new_nbuckets; n++) new_buckets[n].first = new_buckets[n].last = 0; \
	for(n=0; n<self->nbuckets; n++) { \
		while(self->buckets[n].first) { \
			entry_type * entry = self->buckets[n].first; \
			JIVE_LIST_REMOVE(self->buckets[n], entry, chain_member); \
			size_t hashval = jive_str_hash(entry->key_member) & (new_nbuckets - 1); \
			JIVE_LIST_PUSH_BACK(new_buckets[hashval], entry, chain_member); \
		} \
	} \
	 \
	jive_context_free(self->context, self->buckets); \
	self->buckets = new_buckets; \
	self->nbuckets = new_nbuckets; \
	self->mask = new_nbuckets - 1; \
} \
 \
static inline void \
dict_type##_insert(struct dict_type * self, entry_type * entry) \
{ \
	self->nitems ++; \
	if (self->nitems > self->nbuckets) dict_type##_rehash(self); \
	size_t hash = jive_str_hash(entry->key_member) & self->mask; \
	JIVE_LIST_PUSH_BACK(self->buckets[hash], entry, chain_member); \
} \
 \
static inline void \
dict_type##_remove(struct dict_type * self, entry_type * entry) \
{ \
	self->nitems --; \
	size_t hash = jive_str_hash(entry->key_member) & self->mask; \
	JIVE_LIST_REMOVE(self->buckets[hash], entry, chain_member); \
} \
 \
static inline entry_type * \
dict_type##_lookup(const struct dict_type * self, const char keyval[]) \
{ \
	if (!self->nbuckets) return 0; \
	 \
	size_t hash = jive_str_hash(keyval) & self->mask; \
	entry_type * entry = self->buckets[hash].first; \
	while(entry && strcmp(entry->key_member, keyval) != 0) \
		entry = entry->chain_member.next; \
	 \
	return entry; \
} \
 \
static inline void \
dict_type##_iterator_next_bucket(struct dict_type##_iterator * iterator) \
{ \
	while(!iterator->entry && iterator->next_bucket < iterator->hash->nbuckets) { \
		iterator->entry = iterator->hash->buckets[iterator->next_bucket].first; \
		iterator->next_bucket ++; \
	} \
} \
 \
static inline void \
dict_type##_iterator_next(struct dict_type##_iterator * iterator) \
{ \
	iterator->entry = iterator->entry->chain_member.next; \
	if (!iterator->entry) dict_type##_iterator_next_bucket(iterator); \
} \
 \
static inline struct dict_type##_iterator \
dict_type##_begin(const struct dict_type * self) \
{ \
	struct dict_type##_iterator iterator; \
	iterator.hash = self; \
	iterator.entry = 0; \
	iterator.next_bucket = 0; \
	dict_type##_iterator_next_bucket(&iterator); \
	return iterator; \
} \

#define JIVE_DICT_ITERATE(dict_type, hash, iterator) \
	for(iterator = dict_type##_begin(&hash); iterator.entry; dict_type##_iterator_next(&iterator))

#endif
