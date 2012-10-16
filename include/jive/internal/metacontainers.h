/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_INTERNAL_METACONTAINERS_H
#define JIVE_INTERNAL_METACONTAINERS_H

#include <stdbool.h>
#include <string.h>
#include <jive/context.h>

#define DEFINE_SET_TYPE(set_type, value_type) \
\
struct set_type { \
	jive_context * context; \
	value_type * items; \
	size_t nitems, space; \
}; \
 \
static inline void \
set_type##_init(struct set_type * set, jive_context * context) \
{ \
	set->context = context; \
	set->items = 0; \
	set->nitems = set->space = 0; \
} \
 \
static inline void \
set_type##_fini(struct set_type * set) \
{ \
	jive_context_free(set->context, set->items); \
} \
 \
static inline void \
set_type##_enlarge(struct set_type * set, size_t at_least, value_type value) \
{ \
	if (set->space >= at_least) return; \
	size_t new_size = set->space * 2; \
	if (new_size < at_least) new_size = at_least; \
	 \
	set->items = jive_context_realloc(set->context, set->items, new_size * sizeof(value_type)); \
	set->space = new_size; \
} \
 \
static inline bool \
set_type##_contains(const struct set_type * set, value_type value) \
{ \
	size_t low = 0, high = set->nitems; \
	while(low!=high) { \
		size_t mid = (low + high) >> 1; \
		if (set->items[mid] == value) return true; \
		if (set->items[mid] > value) high = mid; \
		else low = mid + 1; \
	} \
	if (low < set->nitems && set->items[low] == value) return true; \
	return false; \
} \
 \
static inline bool \
set_type##_add(struct set_type * set, value_type value) \
{ \
	size_t low = 0, high = set->nitems; \
	while(low!=high) { \
		size_t mid = (low + high) >> 1; \
		if (set->items[mid] == value) return true; \
		if (set->items[mid] > value) high = mid; \
		else low = mid + 1; \
	} \
	if (low < set->nitems && set->items[low] == value) return true; \
	set_type##_enlarge(set, set->nitems+1, value); \
	memmove(&set->items[low+1], &set->items[low], (set->nitems - low) * sizeof(value_type)); \
	set->items[low] = value; \
	set->nitems ++; \
	return false; \
} \
 \
static inline bool \
set_type##_remove(struct set_type * set, value_type value) \
{ \
	size_t low = 0, high = set->nitems; \
	while(low!=high) { \
		size_t mid = (low + high) >> 1; \
		if (set->items[mid] == value) {low = mid; break;} \
		if (set->items[mid] > value) high = mid; \
		else low = mid + 1; \
	} \
	if (low >= set->nitems || set->items[low] != value) return false; \
	set->nitems --; \
	memmove(&set->items[low], &set->items[low+1], (set->nitems - low) * sizeof(set->items[0])); \
	return true ; \
} \
 \
static inline void \
set_type##_clear(struct set_type * set) \
{ \
	set->nitems = 0; \
} \
 \
static inline void \
set_type##_copy(struct set_type * dst, const struct set_type * src) \
{ \
	if (!src->nitems) {set_type##_clear(dst); return;} \
	set_type##_enlarge(dst, src->nitems, src->items[0]); \
	size_t n; \
	for(n=0; n<src->nitems; n++) dst->items[n] = src->items[n]; \
	dst->nitems = src->nitems; \
} \
 \
static inline bool \
set_type##_equals(const struct set_type * first, const struct set_type * second) \
{ \
	if (first->nitems != second->nitems) return false; \
	size_t n; \
	for(n=0; n<first->nitems; n++) \
		if (first->items[n] != second->items[n]) return false; \
	return true; \
} \

#define DEFINE_MULTISET_TYPE(multiset_type, value_type) \
\
struct multiset_type { \
	jive_context * context; \
	struct { value_type value; size_t count; } * items; \
	size_t nitems, space; \
}; \
 \
static inline void \
multiset_type##_init(struct multiset_type * multiset, jive_context * context) \
{ \
	multiset->context = context; \
	multiset->items = 0; \
	multiset->nitems = multiset->space = 0; \
} \
 \
static inline void \
multiset_type##_fini(struct multiset_type * multiset) \
{ \
	jive_context_free(multiset->context, multiset->items); \
} \
 \
static inline void \
multiset_type##_enlarge(struct multiset_type * multiset, size_t at_least, value_type value) \
{ \
	if (multiset->space >= at_least) return; \
	size_t new_size = multiset->space * 2; \
	if (new_size < at_least) new_size = at_least; \
	 \
	multiset->items = jive_context_realloc(multiset->context, multiset->items, new_size * sizeof(multiset->items[0])); \
	multiset->space = new_size; \
} \
 \
static inline size_t \
multiset_type##_contains(const struct multiset_type * multiset, value_type value) \
{ \
	size_t low = 0, high = multiset->nitems; \
	while(low!=high) { \
		size_t mid = (low + high) >> 1; \
		if (multiset->items[mid].value == value) return multiset->items[mid].count; \
		if (multiset->items[mid].value > value) high = mid; \
		else low = mid + 1; \
	} \
	if (low < multiset->nitems && multiset->items[low].value == value) return multiset->items[low].count; \
	return 0; \
} \
 \
static inline size_t \
multiset_type##_add(struct multiset_type * multiset, value_type value) \
{ \
	size_t low = 0, high = multiset->nitems; \
	while(low!=high) { \
		size_t mid = (low + high) >> 1; \
		if (multiset->items[mid].value == value) \
			return multiset->items[mid].count ++; \
		if (multiset->items[mid].value > value) high = mid; \
		else low = mid + 1; \
	} \
	if (low < multiset->nitems && multiset->items[low].value == value) \
		return multiset->items[low].count ++; \
	multiset_type##_enlarge(multiset, multiset->nitems+1, value); \
	memmove(&multiset->items[low+1], &multiset->items[low], (multiset->nitems - low) * sizeof(multiset->items[0])); \
	multiset->items[low].value = value; \
	multiset->items[low].count = 1; \
	multiset->nitems ++; \
	return 0; \
} \
 \
static inline size_t \
multiset_type##_remove(struct multiset_type * multiset, value_type value) \
{ \
	size_t low = 0, high = multiset->nitems; \
	while(low!=high) { \
		size_t mid = (low + high) >> 1; \
		if (multiset->items[mid].value == value) {low = mid; break;} \
		if (multiset->items[mid].value > value) high = mid; \
		else low = mid + 1; \
	} \
	if (low >= multiset->nitems || multiset->items[low].value != value) return 0; \
	size_t count = multiset->items[low].count --; \
	if (count > 1) return count; \
	multiset->nitems --; \
	memmove(&multiset->items[low], &multiset->items[low+1], (multiset->nitems - low) * sizeof(multiset->items[0])); \
	return count ; \
} \
 \
static inline size_t \
multiset_type##_remove_all(struct multiset_type * multiset, value_type value) \
{ \
	size_t low = 0, high = multiset->nitems; \
	while(low!=high) { \
		size_t mid = (low + high) >> 1; \
		if (multiset->items[mid].value == value) {low = mid; break;} \
		if (multiset->items[mid].value > value) high = mid; \
		else low = mid + 1; \
	} \
	if (low >= multiset->nitems || multiset->items[low].value != value) return 0; \
	size_t count = multiset->items[low].count; \
	multiset->nitems --; \
	memmove(&multiset->items[low], &multiset->items[low+1], (multiset->nitems - low) * sizeof(multiset->items[0])); \
	return count ; \
} \
 \
static inline void \
multiset_type##_clear(struct multiset_type * multiset) \
{ \
	multiset->nitems = 0; \
} \
 \
static inline void \
multiset_type##_copy(struct multiset_type * dst, const struct multiset_type * src) \
{ \
	if (!src->nitems) {multiset_type##_clear(dst); return;} \
	multiset_type##_enlarge(dst, src->nitems, src->items[0].value); \
	size_t n; \
	for(n=0; n<src->nitems; n++) dst->items[n] = src->items[n]; \
	dst->nitems = src->nitems; \
} \
 \
static inline bool \
multiset_type##_equals(const struct multiset_type * first, const struct multiset_type * second) \
{ \
	if (first->nitems != second->nitems) return false; \
	size_t n; \
	for(n=0; n<first->nitems; n++) { \
		if (first->items[n].count != second->items[n].count) return false; \
		if (first->items[n].value != second->items[n].value) return false; \
	} \
	return true; \
} \
 \
static inline bool \
multiset_type##_equals_relaxed(const struct multiset_type * first, const struct multiset_type * second) \
{ \
	if (first->nitems != second->nitems) return false; \
	size_t n; \
	for(n=0; n<first->nitems; n++) { \
		if (first->items[n].value != second->items[n].value) return false; \
	} \
	return true; \
} \

#define DEFINE_HASHMAP_TYPE(hashmap_type, key_type, value_type, hash_function) \
 \
struct hashmap_type##_entry { \
	key_type key; \
	value_type value; \
	struct hashmap_type##_entry * next; \
}; \
 \
struct hashmap_type { \
	jive_context * context; \
	size_t nitems, nbuckets; \
	struct hashmap_type##_entry ** buckets; \
	struct hashmap_type##_entry * unused; \
}; \
 \
static inline void \
hashmap_type##_init(struct hashmap_type * map, jive_context * context) \
{ \
	map->context = context; \
	map->nitems = map->nbuckets = 0; \
	map->buckets = 0; \
	map->unused = 0; \
} \
 \
static inline void \
hashmap_type##_fini(struct hashmap_type * map) \
{ \
	size_t n; \
	struct hashmap_type##_entry * entry; \
	for(n=0; n<map->nbuckets; n++) { \
		entry = map->buckets[n]; \
		while(entry) { \
			map->buckets[n] = entry->next; \
			jive_context_free(map->context, entry); \
			entry = map->buckets[n]; \
		} \
	} \
	entry = map->unused; \
	while(entry) { \
		map->unused = entry->next; \
		jive_context_free(map->context, entry); \
		entry = map->unused; \
	} \
	jive_context_free(map->context, map->buckets); \
} \
 \
static inline struct hashmap_type##_entry * \
hashmap_type##_entry_alloc(struct hashmap_type * map, key_type key, value_type value) \
{ \
	struct hashmap_type##_entry * entry = map->unused; \
	if (entry) map->unused = entry->next; \
	else entry = jive_context_malloc(map->context, sizeof(*entry)); \
	entry->key = key; \
	entry->value = value; \
	return entry; \
} \
 \
static inline void \
hashmap_type##_entry_insert(struct hashmap_type * map, struct hashmap_type##_entry * entry, size_t index) \
{ \
	entry->next = map->buckets[index]; \
	map->buckets[index] = entry; \
} \
 \
static inline struct hashmap_type##_entry * \
hashmap_type##_entry_lookup_bucket(const struct hashmap_type * map, key_type key, size_t index) \
{ \
	struct hashmap_type##_entry * current = map->buckets[index]; \
	while(current) { \
		if (current->key == key) return current; \
		current = current->next; \
	} \
	return 0; \
} \
 \
static inline const struct hashmap_type##_entry * \
hashmap_type##_lookup(const struct hashmap_type * map, key_type key) \
{ \
	if (map->nitems == 0) return 0; \
	size_t index = hash_function(key) % map->nbuckets; \
	return hashmap_type##_entry_lookup_bucket(map, key, index); \
} \
 \
static inline void \
hashmap_type##_enlarge(struct hashmap_type * map, key_type new_key, value_type new_value) \
{ \
	struct hashmap_type##_entry ** old_buckets = map->buckets; \
	size_t old_nbuckets = map->nbuckets; \
	 \
	map->nbuckets = 2 * map->nbuckets + 16; \
	map->buckets = jive_context_malloc(map->context, sizeof(*map->buckets) * map->nbuckets); \
	 \
	size_t n; \
	for(n=0; n<map->nbuckets; n++) map->buckets[n] = 0; \
	 \
	for(n=0; n<old_nbuckets; n++) { \
		struct hashmap_type##_entry * current, * next; \
		current = old_buckets[n]; \
		while(current) { \
			next = current->next; \
			size_t index = hash_function(current->key) % map->nbuckets; \
			hashmap_type##_entry_insert(map, current, index); \
			current = next; \
		} \
	} \
	 \
	jive_context_free(map->context, old_buckets); \
} \
 \
static inline bool \
hashmap_type##_set(struct hashmap_type * map, key_type key, value_type value) \
{ \
	size_t index = hash_function(key); \
	struct hashmap_type##_entry * entry; \
	if (map->nitems) { \
		index = index % map->nbuckets; \
		entry = hashmap_type##_entry_lookup_bucket(map, key, index); \
		if (entry) { \
			entry->value = value; \
			return true; \
		} \
	} \
	 \
	entry = hashmap_type##_entry_alloc(map, key, value); \
	if (++map->nitems >= map->nbuckets) {\
		hashmap_type##_enlarge(map, key, value); \
		index = hash_function(key) % map->nbuckets; \
	} \
	hashmap_type##_entry_insert(map, entry, index); \
	return false; \
} \
 \
static inline bool \
hashmap_type##_remove(struct hashmap_type * map, key_type key) \
{ \
	struct hashmap_type##_entry ** location, * current; \
	size_t index = hash_function(key) % map->nbuckets; \
	location = &map->buckets[index]; \
	while( (current = *location) != 0) { \
		if (current->key != key) { \
			location = &current->next; \
			continue; \
		} \
		*location = current->next; \
		current->next = map->unused; \
		map->unused = current; \
		map->nitems--; \
		return true; \
	} \
	return false; \
} \

#endif
