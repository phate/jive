/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_RESOURCE_H
#define JIVE_VSDG_RESOURCE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <jive/common.h>

#include <vector>

struct jive_graph;

namespace jive {
namespace base {
	class type;
}
	class gate;
}

typedef struct jive_resource_class_class jive_resource_class_class;
typedef struct jive_resource_class jive_resource_class;
typedef struct jive_resource_name jive_resource_name;
typedef struct jive_resource_class_count jive_resource_class_count;
typedef struct jive_resource_class_count_item jive_resource_class_count_item;
typedef struct jive_resource_class_count_bucket jive_resource_class_count_bucket;
typedef struct jive_resource_class_demotion jive_resource_class_demotion;
typedef struct jive_rescls_prio_array jive_rescls_prio_array;

typedef enum {
	jive_resource_class_priority_invalid = 0,
	jive_resource_class_priority_control = 1,
	jive_resource_class_priority_reg_implicit = 2,
	jive_resource_class_priority_mem_unique = 3,
	jive_resource_class_priority_reg_high = 4,
	jive_resource_class_priority_reg_low = 5,
	jive_resource_class_priority_mem_generic = 6,
	jive_resource_class_priority_lowest = 7
} jive_resource_class_priority;

struct jive_resource_class {
	const jive_resource_class_class * class_;
	
	const char * name;
	
	/** \brief Upper limit on number of available entities in this class */
	size_t limit;
	
	/** \brief Names of available resources (if limit not 0) */
	const struct jive_resource_name * const * names;
	
	/** \brief Parent resource class */
	const jive_resource_class * parent;
	
	/** \brief Number of steps from root */
	size_t depth;
	
	/** \brief Priority for register allocator */
	jive_resource_class_priority priority;
	
	/** \brief Paths for "demoting" this resource to a different one */
	const jive_resource_class_demotion * demotions;
	
	/** \brief Port and gate type corresponding to this resource */
	const jive::base::type * type;
};

struct jive_resource_class_class {
	const jive_resource_class_class * parent;
	const char * name;
	bool is_abstract;
};

struct jive_resource_class_demotion {
	const jive_resource_class * target;
	const jive_resource_class * const * path;
};

const jive_resource_class *
jive_resource_class_union(const jive_resource_class * self, const jive_resource_class * other);

const jive_resource_class *
jive_resource_class_intersection(const jive_resource_class * self,
	const jive_resource_class * other);

JIVE_EXPORTED_INLINE bool
jive_resource_class_issubclass(const jive_resource_class * self,
	const jive_resource_class * super_class)
{
	while (self) {
		if (self == super_class)
			return true;
		self = self->parent;
	}
	return false;
}

JIVE_EXPORTED_INLINE bool
jive_resource_class_isinstance(const jive_resource_class * self,
	const jive_resource_class_class * cls)
{
	const jive_resource_class_class * tmp = self->class_;
	while (tmp) {
		if (tmp == cls)
			return true;
		tmp = tmp->parent;
	}
	return false;
}

JIVE_EXPORTED_INLINE void
jive_resource_class_get_resource_names(const jive_resource_class * self,
	size_t * count, const jive_resource_name * const ** names)
{
	*count = self->limit;
	*names = self->names;
}

JIVE_EXPORTED_INLINE const jive::base::type *
jive_resource_class_get_type(const jive_resource_class * self)
{
	return self->type;
}

JIVE_EXPORTED_INLINE bool
jive_resource_class_is_abstract(const jive_resource_class * self)
{
	return self->class_->is_abstract;
}

jive::gate *
jive_resource_class_create_gate(const jive_resource_class * self, struct jive_graph * graph,
	const char * name);

/** \brief Find largest resource class of same general type containing this class */
const jive_resource_class *
jive_resource_class_relax(const jive_resource_class * self);

extern const jive_resource_class_class JIVE_ABSTRACT_RESOURCE;
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
	} hash_chain;
	struct {
		jive_resource_class_count_item * prev;
		jive_resource_class_count_item * next;
	} item_list;
};

struct jive_resource_class_count_bucket {
	jive_resource_class_count_item * first;
	jive_resource_class_count_item * last;
};

struct jive_resource_class_count {
	size_t nitems, nbuckets, mask;
	std::vector<jive_resource_class_count_bucket> buckets;
	struct {
		jive_resource_class_count_item * first;
		jive_resource_class_count_item * last;
	} items;
};

struct jive_rescls_prio_array {
	uint16_t count[8];
};

void
jive_rescls_prio_array_compute(jive_rescls_prio_array * self,
	const jive_resource_class_count * count);

int
jive_rescls_prio_array_compare(const jive_rescls_prio_array * self,
	const jive_rescls_prio_array * other);

JIVE_EXPORTED_INLINE void
jive_rescls_prio_array_init(jive_rescls_prio_array * self)
{
	size_t n;
	for (n = 0; n < 8; n++)
		self->count[n] = 0;
}

#endif
