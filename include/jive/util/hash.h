/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_UTIL_HASH_H
#define JIVE_UTIL_HASH_H

#include <stddef.h>

#include <jive/util/list.h>

static inline size_t
jive_ptr_hash(const void * ptr)
{
	/* hm, ideally I would like to "rotate" instead of "shifting"... */
	size_t hash = (size_t) ptr;
	hash ^= (hash >> 20) ^ (hash >> 12);
	return hash ^ (hash >> 7) ^ (hash >> 4);
}

#define JIVE_DECLARE_HASH_TYPE(hash_type, entry_type, key_type, key_member, chain_member) \
struct hash_type##_bucket { \
	entry_type * first; \
	entry_type * last; \
}; \
 \
struct hash_type { \
	struct jive_context * context; \
	size_t nitems, nbuckets, mask; \
	struct hash_type##_bucket * buckets; \
}; \
 \
struct hash_type##_iterator { \
	const struct hash_type * hash; \
	entry_type * entry; \
	size_t next_bucket; \
};

#define JIVE_DEFINE_HASH_TYPE(hash_type, entry_type, key_type, key_member, chain_member) \
 \
static inline void \
hash_type##_init(struct hash_type * self, jive_context * context) \
{ \
	self->context = context; \
	self->nitems = self->nbuckets = self->mask = 0; \
	self->buckets = 0; \
} \
 \
static inline void \
hash_type##_fini(struct hash_type * self) \
{ \
	jive_context_free(self->context, self->buckets); \
} \
 \
static inline void \
hash_type##_rehash(struct hash_type * self) \
{ \
	size_t new_nbuckets = self->nbuckets * 2; \
	if (new_nbuckets == 0) new_nbuckets = 1; \
	struct hash_type##_bucket * new_buckets; \
	new_buckets = jive_context_malloc(self->context, sizeof(*new_buckets) * new_nbuckets); \
	 \
	size_t n; \
	for(n=0; n<new_nbuckets; n++) new_buckets[n].first = new_buckets[n].last = 0; \
	for(n=0; n<self->nbuckets; n++) { \
		while(self->buckets[n].first) { \
			entry_type * entry = self->buckets[n].first; \
			JIVE_LIST_REMOVE(self->buckets[n], entry, chain_member); \
			size_t hashval = jive_ptr_hash(entry->key_member) & (new_nbuckets - 1); \
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
hash_type##_insert(struct hash_type * self, entry_type * entry) \
{ \
	self->nitems ++; \
	if (self->nitems > self->nbuckets) hash_type##_rehash(self); \
	size_t hash = jive_ptr_hash(entry->key_member) & self->mask; \
	JIVE_LIST_PUSH_BACK(self->buckets[hash], entry, chain_member); \
} \
 \
static inline void \
hash_type##_remove(struct hash_type * self, entry_type * entry) \
{ \
	self->nitems --; \
	size_t hash = jive_ptr_hash(entry->key_member) & self->mask; \
	JIVE_LIST_REMOVE(self->buckets[hash], entry, chain_member); \
} \
 \
static inline entry_type * \
hash_type##_lookup(const struct hash_type * self, const key_type keyval) \
{ \
	if (!self->nbuckets) return 0; \
	 \
	size_t hash = jive_ptr_hash(keyval) & self->mask; \
	entry_type * entry = self->buckets[hash].first; \
	while(entry && entry->key_member != keyval) \
		entry = entry->chain_member.next; \
	 \
	return entry; \
} \
 \
static inline void \
hash_type##_iterator_next_bucket(struct hash_type##_iterator * iterator) \
{ \
	while(!iterator->entry && iterator->next_bucket < iterator->hash->nbuckets) { \
		iterator->entry = iterator->hash->buckets[iterator->next_bucket].first; \
		iterator->next_bucket ++; \
	} \
} \
 \
static inline void \
hash_type##_iterator_next(struct hash_type##_iterator * iterator) \
{ \
	iterator->entry = iterator->entry->chain_member.next; \
	if (!iterator->entry) hash_type##_iterator_next_bucket(iterator); \
} \
 \
static inline struct hash_type##_iterator \
hash_type##_begin(const struct hash_type * self) \
{ \
	struct hash_type##_iterator iterator; \
	iterator.hash = self; \
	iterator.entry = 0; \
	iterator.next_bucket = 0; \
	hash_type##_iterator_next_bucket(&iterator); \
	return iterator; \
} \

#define JIVE_HASH_ITERATE(hash_type, hash, iterator) \
	for(iterator = hash_type##_begin(&hash); iterator.entry; hash_type##_iterator_next(&iterator))

#endif
