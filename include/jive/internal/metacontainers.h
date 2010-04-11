#ifndef JIVE_INTERNAL_METACONTAINERS_H
#define JIVE_INTERNAL_METACONTAINERS_H

#include <stdbool.h>
#include <string.h>

#define DEFINE_SET_TYPE(set_type, value_type, reallocator) \
\
struct set_type { \
	value_type * items; \
	size_t nitems, space; \
}; \
 \
static inline void \
set_type##_init(struct set_type * set) \
{ \
	set->items = 0; \
	set->nitems = set->space = 0; \
} \
 \
static inline void \
set_type##_enlarge(struct set_type * set, size_t at_least, value_type value) \
{ \
	if (set->space >= at_least) return; \
	size_t new_size = set->space * 2; \
	if (new_size < at_least) new_size = at_least; \
	 \
	set->items = reallocator(set->items, set->space * sizeof(value_type), new_size * sizeof(value_type), value); \
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

#define DEFINE_MULTISET_TYPE(multiset_type, value_type, reallocator) \
\
struct multiset_type { \
	struct { value_type value; size_t count; } * items; \
	size_t nitems, space; \
}; \
 \
static inline void \
multiset_type##_init(struct multiset_type * multiset) \
{ \
	multiset->items = 0; \
	multiset->nitems = multiset->space = 0; \
} \
 \
static inline void \
multiset_type##_enlarge(struct multiset_type * multiset, size_t at_least, value_type value) \
{ \
	if (multiset->space >= at_least) return; \
	size_t new_size = multiset->space * 2; \
	if (new_size < at_least) new_size = at_least; \
	 \
	multiset->items = reallocator(multiset->items, multiset->space * sizeof(multiset->items[0]), new_size * sizeof(multiset->items[0]), value); \
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

#endif
