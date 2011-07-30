#ifndef JIVE_VSDG_RESOURCE_H
#define JIVE_VSDG_RESOURCE_H

#include <stddef.h>
#include <stdint.h>

struct jive_context;
struct jive_graph;
struct jive_type;

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

struct jive_gate *
jive_resource_class_create_gate(const jive_resource_class * self, struct jive_graph * graph, const char * name);

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
	jive_resource_class_count_bucket * buckets;
	struct {
		jive_resource_class_count_item * first;
		jive_resource_class_count_item * last;
	} items;
	struct jive_context * context;
};

struct jive_rescls_prio_array {
	uint16_t count[8];
};

void
jive_rescls_prio_array_compute(jive_rescls_prio_array * self, const jive_resource_class_count * count);

int
jive_rescls_prio_array_compare(const jive_rescls_prio_array * self, const jive_rescls_prio_array * other);

static inline void
jive_rescls_prio_array_init(jive_rescls_prio_array * self)
{
	size_t n;
	for (n = 0; n < 8; n++)
		self->count[n] = 0;
}

#endif
