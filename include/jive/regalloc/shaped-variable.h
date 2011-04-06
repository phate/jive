#ifndef JIVE_REGALLOC_SHAPED_VARIABLE_H
#define JIVE_REGALLOC_SHAPED_VARIABLE_H

#include <stdbool.h>

#include <jive/regalloc/xpoint.h>
#include <jive/util/hash.h>

typedef struct jive_variable_interference jive_variable_interference;
typedef struct jive_variable_interference_part jive_variable_interference_part;
typedef struct jive_variable_interference_hash jive_variable_interference_hash;

JIVE_DECLARE_HASH_TYPE(jive_variable_interference_hash, struct jive_variable_interference_part, struct jive_variable *, variable, chain);

typedef struct jive_shaped_variable jive_shaped_variable;

struct jive_shaped_graph;
struct jive_variable;

struct jive_resource_name;

JIVE_DECLARE_HASH_TYPE(jive_allowed_resource_names_hash, struct jive_allowed_resource_name, const struct jive_resource_name *, name, chain);
typedef struct jive_allowed_resource_names_hash jive_allowed_resource_names_hash;

struct jive_shaped_variable {
	struct jive_shaped_graph * shaped_graph;
	
	struct jive_variable * variable;
	
	struct {
		jive_shaped_variable * prev;
		jive_shaped_variable * next;
	} hash_chain;
	
	struct {
		jive_shaped_variable * prev;
		jive_shaped_variable * next;
	} assignment_variable_list;
	
	jive_variable_interference_hash interference;
	jive_allowed_resource_names_hash allowed_names;
	size_t squeeze;
};

jive_shaped_variable *
jive_shaped_variable_create(struct jive_shaped_graph * shaped_graph, struct jive_variable * variable);

size_t
jive_shaped_variable_interferes_with(const jive_shaped_variable * self, const jive_shaped_variable * other);

bool
jive_shaped_variable_can_merge(const jive_shaped_variable * self, const struct jive_variable * other);

const struct jive_resource_class *
jive_shaped_variable_check_change_resource_class(const jive_shaped_variable * self, const struct jive_resource_class * new_rescls);

bool
jive_shaped_variable_allowed_resource_name(const jive_shaped_variable * self, const struct jive_resource_name * name);

size_t
jive_shaped_variable_allowed_resource_name_count(const jive_shaped_variable * self);

size_t
jive_shaped_variable_is_active_before(const jive_shaped_variable * self, const struct jive_shaped_node * shaped_node);

size_t
jive_shaped_variable_is_crossing(const jive_shaped_variable * self, const struct jive_shaped_node * shaped_node);

size_t
jive_shaped_variable_is_active_after(const jive_shaped_variable * self, const struct jive_shaped_node * shaped_node);

void
jive_shaped_variable_destroy(jive_shaped_variable * self);

typedef struct jive_shaped_ssavar jive_shaped_ssavar;

struct jive_shaped_graph;
struct jive_ssavar;

struct jive_shaped_ssavar {
	struct jive_shaped_graph * shaped_graph;
	
	struct jive_ssavar * ssavar;
	
	struct {
		jive_shaped_ssavar * prev;
		jive_shaped_ssavar * next;
	} hash_chain;
	
	jive_node_xpoint_hash node_xpoints;
	jive_region_tpoint_hash region_tpoints;
	
	size_t boundary_region_depth;
};

jive_shaped_ssavar *
jive_shaped_ssavar_create(struct jive_shaped_graph * shaped_graph, struct jive_ssavar * ssavar);

void
jive_shaped_ssavar_set_boundary_region_depth(jive_shaped_ssavar * self, size_t depth);

static inline void
jive_shaped_ssavar_lower_boundary_region_depth(jive_shaped_ssavar * self, size_t depth)
{
	if (depth < self->boundary_region_depth)
		jive_shaped_ssavar_set_boundary_region_depth(self, depth);
}

size_t
jive_shaped_ssavar_is_active_before(const jive_shaped_ssavar * self, const struct jive_shaped_node * shaped_node);

size_t
jive_shaped_ssavar_is_crossing(const jive_shaped_ssavar * self, const struct jive_shaped_node * shaped_node);

size_t
jive_shaped_ssavar_is_active_after(const jive_shaped_ssavar * self, const struct jive_shaped_node * shaped_node);

void
jive_shaped_ssavar_destroy(jive_shaped_ssavar * self);

#endif
