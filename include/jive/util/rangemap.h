#ifndef JIVE_UTIL_RANGEMAP
#define JIVE_UTIL_RANGEMAP

#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#define JIVE_DEFINE_RANGEMAP(NAME, VALUE_TYPE, VALUE_TYPE_INITIALIZER) \
 \
struct NAME { \
	VALUE_TYPE * items; \
	ssize_t low; \
	ssize_t high; \
}; \
 \
static inline void \
NAME##_init(struct NAME * self) \
{ \
	self->items = 0; \
	self->low = 0; \
	self->high = 0; \
} \
 \
static inline void \
NAME##_fini(struct NAME * self) \
{ \
	VALUE_TYPE * items = self->items + self->low; \
	free(items); \
} \
 \
static bool \
NAME##_grow_down(struct NAME * self, ssize_t new_low) \
{ \
	VALUE_TYPE * new_items = malloc(sizeof(VALUE_TYPE) * (self->high - new_low)); \
		\
	if (!new_items) \
		return false; \
		\
	new_items = new_items - new_low; \
		\
	ssize_t n; \
	for (n = new_low; n < self->low; n++) \
		new_items[n] = VALUE_TYPE_INITIALIZER; \
	for (n = self->low; n < self->high; n++) \
		new_items[n] = self->items[n]; \
	free(self->items + self->low); \
	self->items = new_items; \
	self->low = new_low; \
	 \
	return true; \
} \
 \
static bool \
NAME##_grow_up(struct NAME * self, ssize_t new_high) \
{ \
	VALUE_TYPE * new_items = realloc( \
		self->items + self->low, \
		sizeof(VALUE_TYPE) * (new_high - self->low)); \
		\
	if (!new_items) \
		return false; \
		\
	new_items = new_items - self->low; \
		\
	ssize_t n; \
	for (n = self->high; n < new_high; n++) \
		new_items[n] = VALUE_TYPE_INITIALIZER; \
	self->items = new_items; \
	self->high = new_high; \
	 \
	return true; \
} \
 \
static inline VALUE_TYPE * \
NAME##_lookup(struct NAME * self, ssize_t index) \
{ \
	if (index < self->low) { \
		if (!NAME##_grow_down(self, index)) \
			return NULL; \
	} \
	 \
	if (index >= self->high) { \
		if (!NAME##_grow_up(self, index + 1)) \
			return NULL; \
	} \
	 \
	return &self->items[index]; \
}

#define JIVE_RANGEMAP_INITIALIZER {NULL, 0, 0}

#endif
