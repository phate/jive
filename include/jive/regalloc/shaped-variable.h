/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_REGALLOC_SHAPED_VARIABLE_H
#define JIVE_REGALLOC_SHAPED_VARIABLE_H

#include <stdbool.h>

#include <unordered_set>

#include <jive/common.h>

#include <jive/regalloc/xpoint.h>
#include <jive/util/hash.h>

struct jive_shaped_variable;
struct jive_variable_interference;

struct jive_variable_interference_part {
	jive_shaped_variable * shaped_variable;
	jive_variable_interference * whole;
private:
	jive::detail::intrusive_hash_anchor<jive_variable_interference_part> hash_chain;
public:
	typedef jive::detail::intrusive_hash_accessor<
		jive_shaped_variable *,
		jive_variable_interference_part,
		&jive_variable_interference_part::shaped_variable,
		&jive_variable_interference_part::hash_chain
	> hash_chain_accessor;
};

struct jive_variable_interference {
	jive_variable_interference_part first;
	jive_variable_interference_part second;
	size_t count;
};

typedef jive::detail::intrusive_hash<
	const jive_shaped_variable *,
	jive_variable_interference_part,
	jive_variable_interference_part::hash_chain_accessor
> jive_variable_interference_hash;

struct jive_shaped_graph;
struct jive_variable;

struct jive_resource_name;

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
	std::unordered_set<const jive_resource_name *> allowed_names;
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

/**
	\brief Determine maximum cross count of this variable
	
	Visit all places at which this variable is alive, determine maximum
	of use counts per register class at these places.
*/
void
jive_shaped_variable_get_cross_count(const jive_shaped_variable * self, jive_resource_class_count * counts);

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
	
	jive_nodevar_xpoint_hash_bynode node_xpoints;
	jive_regvar_xpoint_hash_byregion region_xpoints;
	
	size_t boundary_region_depth;
};

jive_shaped_ssavar *
jive_shaped_ssavar_create(struct jive_shaped_graph * shaped_graph, struct jive_ssavar * ssavar);

void
jive_shaped_ssavar_set_boundary_region_depth(jive_shaped_ssavar * self, size_t depth);

JIVE_EXPORTED_INLINE void
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
