#ifndef JIVE_VSDG_RESOURCE_H
#define JIVE_VSDG_RESOURCE_H

#include <stddef.h>

struct jive_type;

typedef struct jive_resource_class jive_resource_class;
typedef struct jive_resource_name jive_resource_name;
typedef struct jive_resource_class_count jive_resource_class_count;
typedef struct jive_resource_class_count_item jive_resource_class_count_item;
typedef struct jive_resource_class_count_bucket jive_resource_class_count_bucket;
typedef struct jive_resource_class_demotion jive_resource_class_demotion;

typedef enum {
	jive_resource_class_priority_invalid = 0,
	jive_resource_class_priority_control = 1,
	jive_resource_class_priority_reg_implicit = 2,
	jive_resource_class_priority_reg_high = 3,
	jive_resource_class_priority_reg_low = 4,
	jive_resource_class_priority_mem_high = 5,
	jive_resource_class_priority_mem_low = 6,
	jive_resource_class_priority_lowest = 7
} jive_resource_class_priority;

struct jive_resource_class {
	const char * name;
	
	/** \brief Upper limit on number of available entities in this class */
	size_t limit;
	
	/** \brief Names of available resources (if limit not 0) */
	const struct jive_resource_name * const * names;
	
	/** \brief Parent resource class */
	const jive_resource_class * parent;
	
	/** \brief Number of step from root */
	size_t depth;
	
	/** \brief Priority for register allocator */
	jive_resource_class_priority priority;
	
	/** \brief Paths for "demoting" this resource to a different one */
	const jive_resource_class_demotion * demotions;
	
	/** \brief Port and gate type corresponding to this resource */
	const struct jive_type * type;
};

struct jive_resource_class_demotion {
	const jive_resource_class * target;
	const jive_resource_class * const * path;
};

const jive_resource_class *
jive_resource_class_union(const jive_resource_class * self, const jive_resource_class * other);

const jive_resource_class *
jive_resource_class_intersection(const jive_resource_class * self, const jive_resource_class * other);

static inline void
jive_resource_class_get_resource_names(const jive_resource_class * self,
	size_t * count, const jive_resource_name * const ** names)
{
	*count = self->limit;
	*names = self->names;
}

static inline const struct jive_type *
jive_resource_class_get_type(const jive_resource_class * self)
{
	return self->type;
}

/** \brief Find largest resource class of same general type containing this class */
const jive_resource_class *
jive_resource_class_relax(const jive_resource_class * self);

extern const jive_resource_class jive_root_resource_class;

struct jive_resource_name {
	const char * name;
	const jive_resource_class * resource_class;
};

struct jive_resource_class_count_item {
	const struct jive_resource_class * resource_class;
	size_t count;
	struct {
		jive_resource_class_count_item * prev;
		jive_resource_class_count_item * next;
	} chain;
};

struct jive_resource_class_count_bucket {
	jive_resource_class_count_item * first;
	jive_resource_class_count_item * last;
};

struct jive_resource_class_count {
	size_t nitems, nbuckets;
	jive_resource_class_count_bucket * buckets;
};

#endif
