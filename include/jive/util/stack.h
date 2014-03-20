/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_UTIL_STACK_H
#define JIVE_UTIL_STACK_H

#include <stddef.h>

#include <jive/common.h>
#include <jive/context.h>
#include <jive/util/list.h>

#define JIVE_DECLARE_STACK_TYPE(stack_type, item_type) \
\
typedef struct stack_type stack_type; \
struct stack_type { \
	struct jive_context * context; \
	item_type * items; \
	size_t nitems, size; \
}; \

#define JIVE_DEFINE_STACK_TYPE(stack_type, item_type) \
\
static inline void \
stack_type##_init(struct stack_type * self, struct jive_context * context) \
{ \
	self->context = context; \
	self->items = 0; \
	self->nitems = 0; \
	self->size = 0; \
} \
\
static inline void \
stack_type##_pop(struct stack_type * self) \
{ \
	if (self->nitems > 0) \
		self->nitems--; \
} \
\
static inline void \
stack_type##_fini(struct stack_type * self) \
{ \
	jive_context_free(self->context, self->items); \
} \
\
static inline void \
stack_type##_push(struct stack_type * self, const item_type item) \
{ \
	if (self->size == self->nitems) { \
		size_t new_size = 2 * self->size + 1; \
		self->items = jive_context_realloc(self->context, self->items, new_size * sizeof(item_type)); \
		self->size = new_size; \
	} \
	self->items[self->nitems++] = item; \
} \
\
static inline item_type \
stack_type##_top(struct stack_type * self) \
{ \
	JIVE_DEBUG_ASSERT(self->nitems != 0); \
	return self->items[self->nitems-1]; \
} \
\
static inline size_t \
stack_type##_size(const struct stack_type * self) \
{ \
	return self->nitems; \
} \

#endif
