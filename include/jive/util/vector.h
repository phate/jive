/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_UTIL_VECTOR_H
#define JIVE_UTIL_VECTOR_H

#include <stddef.h>

#include <jive/common.h>

struct jive_context;

#define JIVE_DECLARE_VECTOR_TYPE(vector_type, value_type) \
 \
typedef struct vector_type vector_type; \
struct vector_type { \
	value_type * items; \
	size_t size, space; \
};

#define JIVE_DEFINE_VECTOR_TYPE(vector_type, value_type) \
 \
static inline void \
vector_type##_init(struct vector_type * self) \
{ \
	self->items = 0; \
	self->size = 0; \
	self->space = 0; \
} \
 \
static inline void \
vector_type##_fini(struct vector_type * self, jive_context * context) \
{ \
	jive_context_free(context, self->items); \
} \
 \
static inline void \
vector_type##_clear(struct vector_type * self) \
{ \
	self->size = 0; \
} \
 \
static inline size_t \
vector_type##_size(const struct vector_type * self) \
{ \
	return self->size; \
} \
 \
static inline value_type \
vector_type##_item(const struct vector_type * self, size_t index) \
{ \
	JIVE_DEBUG_ASSERT(index < self->size); \
	return self->items[index]; \
} \
 \
static inline void \
vector_type##_push_back(struct vector_type * self, jive_context * context, value_type value) \
{ \
	if (self->size == self->space) { \
		size_t new_space = 2 * self->space + 1; \
		self->items = jive_context_realloc(context, self->items, \
			new_space * sizeof(value_type)); \
		self->space = new_space; \
	} \
	self->items[self->size] = value; \
	self->size ++; \
} \
\
static inline void \
vector_type##_swap(struct vector_type * self, size_t index1, size_t index2) \
{ \
	value_type tmp = vector_type##_item(self, index1); \
	self->items[index1] = self->items[index2]; \
	self->items[index2] = tmp; \
} \
\
static inline bool \
vector_type##_is_empty(const struct vector_type * self) \
{ \
	return vector_type##_size(self) == 0; \
}

#endif
