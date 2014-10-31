/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_REGALLOC_XPOINT_H
#define JIVE_REGALLOC_XPOINT_H

#include <jive/util/intrusive-hash.h>
#include <jive/vsdg/resource.h>

namespace jive {
	class output;
}

typedef struct jive_cutvar_xpoint jive_cutvar_xpoint;
typedef struct jive_nodevar_xpoint jive_nodevar_xpoint;
typedef struct jive_regvar_xpoint jive_regvar_xpoint;

struct jive_shaped_node;
struct jive_shaped_region;
struct jive_shaped_ssavar;

struct jive_variable;

struct jive_nodevar_xpoint {
	jive_shaped_node * shaped_node;
	jive_shaped_ssavar * shaped_ssavar;
	
	size_t before_count;
	size_t cross_count;
	size_t after_count;

private:
	jive::detail::intrusive_hash_anchor<jive_nodevar_xpoint> node_hash_chain;
	jive::detail::intrusive_hash_anchor<jive_nodevar_xpoint> ssavar_hash_chain;

public:
	typedef jive::detail::intrusive_hash_accessor<
		jive_shaped_node *,
		jive_nodevar_xpoint,
		&jive_nodevar_xpoint::shaped_node,
		&jive_nodevar_xpoint::node_hash_chain
	> node_hash_accessor;
	typedef jive::detail::intrusive_hash_accessor<
		jive_shaped_ssavar *,
		jive_nodevar_xpoint,
		&jive_nodevar_xpoint::shaped_ssavar,
		&jive_nodevar_xpoint::ssavar_hash_chain
	> ssavar_hash_accessor;
};

typedef jive::detail::intrusive_hash<
	const jive_shaped_node *,
	jive_nodevar_xpoint,
	jive_nodevar_xpoint::node_hash_accessor
> jive_nodevar_xpoint_hash_bynode;

typedef jive::detail::intrusive_hash<
	const jive_shaped_ssavar *,
	jive_nodevar_xpoint,
	jive_nodevar_xpoint::ssavar_hash_accessor
> jive_nodevar_xpoint_hash_byssavar;

struct jive_cutvar_xpoint {
	jive_shaped_ssavar * shaped_ssavar;
	jive::output * origin;
	jive_variable * variable;
	const jive_resource_class * rescls;
	
	size_t count;
	
	struct {
		struct jive_cutvar_xpoint * prev;
		struct jive_cutvar_xpoint * next;
	} varcut_xpoints_list;

	jive::detail::intrusive_hash_anchor<jive_cutvar_xpoint> ssavar_hash_chain;
	jive::detail::intrusive_hash_anchor<jive_cutvar_xpoint> origin_hash_chain;
	jive::detail::intrusive_hash_anchor<jive_cutvar_xpoint> variable_hash_chain;

	typedef jive::detail::intrusive_hash_accessor<
		jive_shaped_ssavar *,
		jive_cutvar_xpoint,
		&jive_cutvar_xpoint::shaped_ssavar,
		&jive_cutvar_xpoint::ssavar_hash_chain
	> ssavar_hash_accessor;
	typedef jive::detail::intrusive_hash_accessor<
		jive::output *,
		jive_cutvar_xpoint,
		&jive_cutvar_xpoint::origin,
		&jive_cutvar_xpoint::origin_hash_chain
	> origin_hash_accessor;
	typedef jive::detail::intrusive_hash_accessor<
		jive_variable *,
		jive_cutvar_xpoint,
		&jive_cutvar_xpoint::variable,
		&jive_cutvar_xpoint::variable_hash_chain
	> variable_hash_accessor;
};

typedef jive::detail::intrusive_hash<
	const jive_shaped_ssavar *,
	jive_cutvar_xpoint,
	jive_cutvar_xpoint::ssavar_hash_accessor
> jive_cutvar_xpoint_hash_byssavar;
typedef jive::detail::intrusive_hash<
	const jive::output *,
	jive_cutvar_xpoint,
	jive_cutvar_xpoint::origin_hash_accessor
> jive_cutvar_xpoint_hash_byorigin;
typedef jive::detail::intrusive_hash<
	const jive_variable *,
	jive_cutvar_xpoint,
	jive_cutvar_xpoint::variable_hash_accessor
> jive_cutvar_xpoint_hash_byvariable;

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

typedef struct jive_varcut jive_mutable_varcut;
typedef struct jive_varcut jive_varcut;

struct jive_regvar_xpoint {
	jive_cutvar_xpoint base;
	
	jive_shaped_region * shaped_region;

private:
	jive::detail::intrusive_hash_anchor<jive_regvar_xpoint> region_hash_chain;

public:
	typedef jive::detail::intrusive_hash_accessor<
		jive_shaped_region *,
		jive_regvar_xpoint,
		&jive_regvar_xpoint::shaped_region,
		&jive_regvar_xpoint::region_hash_chain
	> region_hash_accessor;
};

typedef jive::detail::intrusive_hash<
	const jive_shaped_region *,
	jive_regvar_xpoint,
	jive_regvar_xpoint::region_hash_accessor
> jive_regvar_xpoint_hash_byregion;

struct jive_region_varcut {
	jive_varcut base;
	jive_shaped_region * shaped_region;
};

typedef struct jive_region_varcut jive_region_varcut;

#endif
