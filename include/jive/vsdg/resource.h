#ifndef JIVE_VSDG_RESOURCE_H
#define JIVE_VSDG_RESOURCE_H

#include <stddef.h>

typedef struct jive_resource_class jive_resource_class;
typedef struct jive_resource_name jive_resource_name;
typedef struct jive_resource_class_count jive_resource_class_count;
typedef struct jive_resource_class_count_item jive_resource_class_count_item;
typedef struct jive_resource_class_count_bucket jive_resource_class_count_bucket;

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
