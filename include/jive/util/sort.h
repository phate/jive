/*
 * Copyright 2013 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_UTIL_SORT_H
#define JIVE_UTIL_SORT_H

#include <jive/util/heap.h>

#define DEFINE_BUBBLESORT(container_type, obj_type, comparator) \
 \
/* sort array using bubblesort (suitable for small arrays) */ \
static inline void \
container_type##_bubblesort(obj_type * vec, size_t count) \
{ \
	size_t n, k; \
	for (n = 1; n < count; ++n) { \
		for (k = 0; k < n; ++k) { \
			if (comparator(vec[k+1], vec[k])) { \
				obj_type tmp = vec[k]; \
				vec[k] = vec[k+1]; \
				vec[k+1] = tmp; \
			} \
		} \
	} \
}

#define DEFINE_HEAPSORT(container_type, obj_type, comparator) \
 \
DEFINE_HEAP(container_type##_heap, obj_type, comparator) \
 \
/* apply heap sort to array */ \
static inline void \
container_type##_heapsort(obj_type * vec, size_t size) \
{ \
	size_t n; \
	for (n = 0; n < size; ++n) { \
		container_type##_heap_push(vec, n, vec[n]); \
	} \
	for (n = size; n > 0; --n) { \
		obj_type tmp = vec[0]; \
		container_type##_heap_pop(vec, n); \
		vec[n - 1] = tmp; \
	} \
}

#define DEFINE_SORT(container_type, obj_type, comparator) \
 \
DEFINE_HEAPSORT(container_type, obj_type, comparator) \
DEFINE_BUBBLESORT(container_type, obj_type, comparator) \
 \
/* sort array using some "reasonable" strategy */ \
static inline void \
container_type##_sort(obj_type * vec, size_t count) \
{ \
	if (count > 10) \
		container_type##_heapsort(vec, count); \
	else \
		container_type##_bubblesort(vec, count); \
}

#endif
