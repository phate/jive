/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_REGALLOC_XPOINT_PRIVATE_H
#define JIVE_REGALLOC_XPOINT_PRIVATE_H

#include <jive/context.h>
#include <jive/regalloc/xpoint.h>

jive_nodevar_xpoint *
jive_nodevar_xpoint_create(
	jive_shaped_node * shaped_node,
	jive_shaped_ssavar *
	shaped_ssavar);

void
jive_nodevar_xpoint_destroy(jive_nodevar_xpoint * self);

static inline void
jive_nodevar_xpoint_put(jive_nodevar_xpoint * self)
{
	if (self->before_count == 0 && self->after_count == 0)
		jive_nodevar_xpoint_destroy(self);
}

jive_cutvar_xpoint *
jive_cutvar_xpoint_create(jive_shaped_ssavar * shaped_ssavar);

void
jive_cutvar_xpoint_destroy(jive_cutvar_xpoint * self);

static inline void
jive_cutvar_xpoint_put(jive_cutvar_xpoint * self)
{
	if (self->count == 0)
		jive_cutvar_xpoint_destroy(self);
}

void
jive_varcut_init(jive_varcut * self, jive_context * context);

void
jive_varcut_fini(jive_varcut * self);

jive_shaped_ssavar *
jive_varcut_map_output(const jive_varcut * self, jive::output * output);

jive_shaped_ssavar *
jive_varcut_map_variable(const jive_varcut * self, jive_variable * variable);

static inline size_t
jive_varcut_shaped_ssavar_is_active(const jive_varcut * self, jive_shaped_ssavar * shaped_ssavar)
{
	auto i = self->ssavar_map.find(shaped_ssavar);
	if (i != self->ssavar_map.end()) {
		return i->count;
	} else {
		return 0;
	}
}

static inline size_t
jive_varcut_output_is_active(const jive_varcut * self, jive::output * output)
{
	auto i = self->origin_map.find(output);
	if (i != self->origin_map.end()) {
		return i->count;
	} else {
		return 0;
	}
}

void
jive_mutable_varcut_clear(jive_mutable_varcut * self);

void
jive_mutable_varcut_assign(jive_mutable_varcut * self, const jive_varcut * other);

size_t
jive_mutable_varcut_ssavar_add(
	jive_mutable_varcut * self,
	jive_shaped_ssavar * shaped_ssavar,
	size_t count);

size_t
jive_mutable_varcut_ssavar_remove(
	jive_mutable_varcut * self,
	jive_shaped_ssavar * shaped_ssavar,
	size_t count);

void
jive_mutable_varcut_ssavar_remove_full(
	jive_mutable_varcut * self,
	jive_shaped_ssavar * shaped_ssavar);

void
jive_mutable_varcut_ssavar_divert_origin(
	jive_mutable_varcut * self,
	jive_shaped_ssavar * shaped_ssavar,
	jive::output * origin);

void
jive_mutable_varcut_ssavar_variable_change(
	jive_mutable_varcut * self,
	jive_shaped_ssavar * shaped_ssavar,
	jive_variable * variable);

void
jive_mutable_varcut_ssavar_rescls_change(
	jive_mutable_varcut * self,
	jive_shaped_ssavar * shaped_ssavar,
	const jive_resource_class * rescls);

void
jive_region_varcut_init(jive_region_varcut * self, jive_shaped_region * shaped_region);

void
jive_region_varcut_fini(jive_region_varcut * self);

size_t
jive_region_varcut_ssavar_add(
	jive_region_varcut * self,
	jive_shaped_ssavar * shaped_ssavar,
	size_t count);

size_t
jive_region_varcut_ssavar_remove(
	jive_region_varcut * self,
	jive_shaped_ssavar * shaped_ssavar,
	size_t count);

void
jive_region_varcut_ssavar_divert_origin(jive_region_varcut * self,
	jive_shaped_ssavar * shaped_ssavar, jive::output * origin);

void
jive_region_varcut_ssavar_variable_change(
	jive_region_varcut * self,
	jive_shaped_ssavar * shaped_ssavar,
	jive_variable * variable);

void
jive_region_varcut_ssavar_rescls_change(
	jive_region_varcut * self,
	jive_shaped_ssavar * shaped_ssavar,
	const jive_resource_class * rescls);

static inline size_t
jive_region_varcut_output_is_active(const jive_region_varcut * self, jive::output * output)
{
	return jive_varcut_output_is_active(&self->base, output);
}

#endif
