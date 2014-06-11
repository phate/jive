/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_REGALLOC_XPOINT_H
#define JIVE_REGALLOC_XPOINT_H

#include <jive/util/hash.h>

#include <jive/vsdg/resource.h>

namespace jive {
	class output;
}

typedef struct jive_nodevar_xpoint jive_nodevar_xpoint;
typedef struct jive_cutvar_xpoint jive_cutvar_xpoint;
typedef struct jive_regvar_xpoint jive_regvar_xpoint;

struct jive_shaped_node;
struct jive_shaped_region;
struct jive_shaped_ssavar;

struct jive_nodevar_xpoint {
	struct jive_shaped_node * shaped_node;
	struct jive_shaped_ssavar * shaped_ssavar;
	
	size_t before_count;
	size_t cross_count;
	size_t after_count;
	
	struct {
		struct jive_nodevar_xpoint * prev;
		struct jive_nodevar_xpoint * next;
	} node_hash_chain;
	
	struct {
		struct jive_nodevar_xpoint * prev;
		struct jive_nodevar_xpoint * next;
	} ssavar_hash_chain;
};

JIVE_DECLARE_HASH_TYPE(jive_nodevar_xpoint_hash_bynode, jive_nodevar_xpoint, struct jive_shaped_node *, shaped_node, node_hash_chain);
JIVE_DECLARE_HASH_TYPE(jive_nodevar_xpoint_hash_byssavar, jive_nodevar_xpoint, struct jive_shaped_ssavar *, shaped_ssavar, ssavar_hash_chain);

typedef struct jive_nodevar_xpoint_hash_bynode jive_nodevar_xpoint_hash_bynode;
typedef struct jive_nodevar_xpoint_hash_byssavar jive_nodevar_xpoint_hash_byssavar;

struct jive_cutvar_xpoint {
	struct jive_shaped_ssavar * shaped_ssavar;
	jive::output * origin;
	struct jive_variable * variable;
	const struct jive_resource_class * rescls;
	
	size_t count;
	
	struct {
		struct jive_cutvar_xpoint * prev;
		struct jive_cutvar_xpoint * next;
	} varcut_xpoints_list;
	
	struct {
		struct jive_cutvar_xpoint * prev;
		struct jive_cutvar_xpoint * next;
	} ssavar_hash_chain;
	
	struct {
		struct jive_cutvar_xpoint * prev;
		struct jive_cutvar_xpoint * next;
	} origin_hash_chain;
	
	struct {
		struct jive_cutvar_xpoint * prev;
		struct jive_cutvar_xpoint * next;
	} variable_hash_chain;
};

JIVE_DECLARE_HASH_TYPE(jive_cutvar_xpoint_hash_byssavar, jive_cutvar_xpoint, struct jive_shaped_ssavar *, shaped_ssavar, ssavar_hash_chain);
JIVE_DECLARE_HASH_TYPE(jive_cutvar_xpoint_hash_byorigin, jive_cutvar_xpoint, jive::output *, origin,
	origin_hash_chain);
JIVE_DECLARE_HASH_TYPE(jive_cutvar_xpoint_hash_byvariable, jive_cutvar_xpoint, struct jive_variable *, variable, variable_hash_chain);

typedef struct jive_regvar_xpoint_hash_byregion jive_regvar_xpoint_hash_byregion;
typedef struct jive_cutvar_xpoint_hash_byssavar jive_cutvar_xpoint_hash_byssavar;
typedef struct jive_cutvar_xpoint_hash_byorigin jive_cutvar_xpoint_hash_byorigin;
typedef struct jive_cutvar_xpoint_hash_byvariable jive_cutvar_xpoint_hash_byvariable;

struct jive_varcut {
	jive_cutvar_xpoint_hash_byssavar ssavar_map;
	jive_cutvar_xpoint_hash_byorigin origin_map;
	jive_cutvar_xpoint_hash_byvariable variable_map;
	jive_resource_class_count use_counts;
	struct jive_context * context;
	
	struct {
		jive_cutvar_xpoint * first;
		jive_cutvar_xpoint * last;
	} xpoints;
};

typedef struct jive_varcut jive_varcut;
typedef struct jive_varcut jive_mutable_varcut;

struct jive_regvar_xpoint {
	jive_cutvar_xpoint base;
	
	struct jive_shaped_region * shaped_region;
	
	struct {
		struct jive_regvar_xpoint * prev;
		struct jive_regvar_xpoint * next;
	} region_hash_chain;
};

JIVE_DECLARE_HASH_TYPE(jive_regvar_xpoint_hash_byregion, jive_regvar_xpoint, struct jive_shaped_region *, shaped_region, region_hash_chain);

struct jive_region_varcut {
	jive_varcut base;
	struct jive_shaped_region * shaped_region;
};

typedef struct jive_region_varcut jive_region_varcut;

#endif
