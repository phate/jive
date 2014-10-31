/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/regalloc/xpoint-private.h>

#include <jive/common.h>

#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/shaped-node.h>
#include <jive/regalloc/shaped-variable.h>
#include <jive/util/list.h>
#include <jive/vsdg/basetype.h>
#include <jive/vsdg/resource-private.h>
#include <jive/vsdg/variable.h>

jive_nodevar_xpoint *
jive_nodevar_xpoint_create(jive_shaped_node * shaped_node, jive_shaped_ssavar * shaped_ssavar)
{
	jive_nodevar_xpoint * xpoint = new jive_nodevar_xpoint;
	xpoint->shaped_node = shaped_node;
	xpoint->shaped_ssavar = shaped_ssavar;
	xpoint->before_count = 0;
	xpoint->cross_count = 0;
	xpoint->after_count = 0;
	
	shaped_node->ssavar_xpoints.insert(xpoint);
	shaped_ssavar->node_xpoints.insert(xpoint);
	
	return xpoint;
}

void
jive_nodevar_xpoint_destroy(jive_nodevar_xpoint * xpoint)
{
	jive_shaped_node * shaped_node = xpoint->shaped_node;
	jive_shaped_ssavar * shaped_ssavar = xpoint->shaped_ssavar;
	jive_context * context = shaped_node->shaped_graph->context;
	
	shaped_node->ssavar_xpoints.erase(xpoint);
	shaped_ssavar->node_xpoints.erase(xpoint);
	
	delete xpoint;
}

void
jive_varcut_init(jive_varcut * self, jive_context * context)
{
	self->context = context;
	jive_resource_class_count_init(&self->use_counts, context);
	self->xpoints.first = self->xpoints.last = 0;
}

void
jive_varcut_fini(jive_varcut * self)
{
	jive_mutable_varcut_clear(self);
	jive_resource_class_count_fini(&self->use_counts);
}

jive_shaped_ssavar *
jive_varcut_map_output(const jive_varcut * self, jive::output * output)
{
	auto i = self->origin_map.find(output);
	if (i != self->origin_map.end()) {
		return i->shaped_ssavar;
	} else {
		return nullptr;
	}
}

jive_shaped_ssavar *
jive_varcut_map_variable(const jive_varcut * self, jive_variable * variable)
{
	auto i = self->variable_map.find(variable);
	if (i != self->variable_map.end()) {
		return i->shaped_ssavar;
	} else {
		return nullptr;
	}
}

static void
jive_mutable_varcut_register_xpoint(jive_mutable_varcut * self, jive_cutvar_xpoint * xpoint)
{
	self->ssavar_map.insert(xpoint);
	self->origin_map.insert(xpoint);
	self->variable_map.insert(xpoint);
	JIVE_LIST_PUSH_BACK(self->xpoints, xpoint, varcut_xpoints_list);
	jive_resource_class_count_add(&self->use_counts, xpoint->rescls);
}

static void
jive_mutable_varcut_unregister_xpoint(jive_mutable_varcut * self, jive_cutvar_xpoint * xpoint)
{
	self->ssavar_map.erase(xpoint);
	self->origin_map.erase(xpoint);
	self->variable_map.erase(xpoint);
	JIVE_LIST_REMOVE(self->xpoints, xpoint, varcut_xpoints_list);
	jive_resource_class_count_sub(&self->use_counts, xpoint->rescls);
}

void
jive_mutable_varcut_clear(jive_mutable_varcut * self)
{
	jive_resource_class_count_clear(&self->use_counts);
	
	jive_cutvar_xpoint * xpoint, * next_xpoint;
	JIVE_LIST_ITERATE_SAFE(self->xpoints, xpoint, next_xpoint, varcut_xpoints_list) {
		self->ssavar_map.erase(xpoint);
		self->origin_map.erase(xpoint);
		self->variable_map.erase(xpoint);
		JIVE_LIST_REMOVE(self->xpoints, xpoint, varcut_xpoints_list);
		
		delete xpoint;
	}
}

void
jive_mutable_varcut_assign(jive_mutable_varcut * self, const jive_varcut * other)
{
	jive_mutable_varcut_clear(self);
	
	const jive_cutvar_xpoint * src_xpoint;
	JIVE_LIST_ITERATE(other->xpoints, src_xpoint, varcut_xpoints_list) {
		jive_cutvar_xpoint * xpoint = new jive_cutvar_xpoint;
		xpoint->shaped_ssavar = src_xpoint->shaped_ssavar;
		xpoint->origin = src_xpoint->origin;
		xpoint->variable = src_xpoint->variable;
		xpoint->rescls = src_xpoint->rescls;
		xpoint->count = src_xpoint->count;
		jive_mutable_varcut_register_xpoint(self, xpoint);
	}
}

size_t
jive_mutable_varcut_ssavar_add(
	jive_mutable_varcut * self,
	jive_shaped_ssavar * shaped_ssavar,
	size_t count)
{
	if (!count)
		return 0;
	
	auto i = self->ssavar_map.find(shaped_ssavar);
	
	jive_cutvar_xpoint * xpoint;
	if (i == self->ssavar_map.end()) {
		xpoint = new jive_cutvar_xpoint;
		xpoint->shaped_ssavar = shaped_ssavar;
		xpoint->origin = shaped_ssavar->ssavar->origin;
		xpoint->variable = shaped_ssavar->ssavar->variable;
		xpoint->rescls = jive_variable_get_resource_class(shaped_ssavar->ssavar->variable);
		xpoint->count = 0;
		jive_mutable_varcut_register_xpoint(self, xpoint);
	} else {
		xpoint = i.ptr();
	}
	
	size_t old_count = xpoint->count;
	xpoint->count += count;
	return old_count;
}

size_t
jive_mutable_varcut_ssavar_remove(
	jive_mutable_varcut * self,
	jive_shaped_ssavar * shaped_ssavar,
	size_t count)
{
	auto i = self->ssavar_map.find(shaped_ssavar);
	if (i == self->ssavar_map.end()) {
		return 0;
	}
	jive_cutvar_xpoint * xpoint = i.ptr();
	
	size_t old_count = xpoint->count;
	xpoint->count -= count;
	if (xpoint->count == 0) {
		jive_mutable_varcut_unregister_xpoint(self, xpoint);
		delete xpoint;
	}
	
	return old_count;
}

void
jive_mutable_varcut_ssavar_remove_full(
	jive_mutable_varcut * self,
	jive_shaped_ssavar * shaped_ssavar)
{
	auto i = self->ssavar_map.find(shaped_ssavar);
	if (i == self->ssavar_map.end()) {
		return;
	}
	jive_cutvar_xpoint * xpoint = i.ptr();
	
	jive_mutable_varcut_unregister_xpoint(self, xpoint);
	delete xpoint;
}

void
jive_mutable_varcut_ssavar_divert_origin(jive_mutable_varcut * self,
	jive_shaped_ssavar * shaped_ssavar, jive::output * origin)
{
	auto i = self->ssavar_map.find(shaped_ssavar);
	if (i == self->ssavar_map.end()) {
		return;
	}
	jive_cutvar_xpoint * xpoint = i.ptr();
	self->origin_map.erase(xpoint);
	xpoint->origin = origin;
	self->origin_map.insert(xpoint);
}

void
jive_mutable_varcut_ssavar_variable_change(
	jive_mutable_varcut * self,
	jive_shaped_ssavar * shaped_ssavar,
	jive_variable * variable)
{
	auto i = self->ssavar_map.find(shaped_ssavar);
	if (i == self->ssavar_map.end()) {
		return;
	}
	jive_cutvar_xpoint * xpoint = i.ptr();
	
	self->variable_map.erase(xpoint);
	xpoint->variable = variable;
	self->variable_map.insert(xpoint);
}


void
jive_mutable_varcut_ssavar_rescls_change(
	jive_mutable_varcut * self,
	jive_shaped_ssavar * shaped_ssavar,
	const jive_resource_class * rescls)
{
	auto i = self->ssavar_map.find(shaped_ssavar);
	if (i == self->ssavar_map.end()) {
		return;
	}
	jive_cutvar_xpoint * xpoint = i.ptr();
	
	jive_resource_class_count_change(&self->use_counts, xpoint->rescls, rescls);
	xpoint->rescls = rescls;
}


static void
jive_region_varcut_register_xpoint(jive_region_varcut * self, jive_regvar_xpoint * xpoint)
{
	self->base.ssavar_map.insert(&xpoint->base);
	self->base.origin_map.insert(&xpoint->base);
	self->base.variable_map.insert(&xpoint->base);
	xpoint->base.shaped_ssavar->region_xpoints.insert(xpoint);
	JIVE_LIST_PUSH_BACK(self->base.xpoints, &xpoint->base, varcut_xpoints_list);
	jive_resource_class_count_add(&self->base.use_counts, xpoint->base.rescls);
}

static void
jive_region_varcut_unregister_xpoint(jive_region_varcut * self, jive_regvar_xpoint * xpoint)
{
	self->base.ssavar_map.erase(&xpoint->base);
	self->base.origin_map.erase(&xpoint->base);
	self->base.variable_map.erase(&xpoint->base);
	xpoint->base.shaped_ssavar->region_xpoints.erase(xpoint);
	JIVE_LIST_REMOVE(self->base.xpoints, &xpoint->base, varcut_xpoints_list);
	jive_resource_class_count_add(&self->base.use_counts, xpoint->base.rescls);
}


void
jive_region_varcut_init(jive_region_varcut * self, jive_shaped_region * shaped_region)
{
	jive_varcut_init(&self->base, shaped_region->shaped_graph->context);
	self->shaped_region = shaped_region;
}

void
jive_region_varcut_fini(jive_region_varcut * self)
{
	JIVE_DEBUG_ASSERT(self->base.xpoints.first == 0 && self->base.xpoints.last == 0);
	
	jive_resource_class_count_fini(&self->base.use_counts);
}

size_t
jive_region_varcut_ssavar_add(
	jive_region_varcut * self,
	jive_shaped_ssavar * shaped_ssavar,
	size_t count)
{
	if (!count)
		return 0;
	
	jive_regvar_xpoint * xpoint;
	auto i = self->base.ssavar_map.find(shaped_ssavar);
	
	if (i == self->base.ssavar_map.end()) {
		xpoint = new jive_regvar_xpoint;
		xpoint->base.shaped_ssavar = shaped_ssavar;
		xpoint->base.origin = shaped_ssavar->ssavar->origin;
		xpoint->base.variable = shaped_ssavar->ssavar->variable;
		xpoint->base.rescls = jive_variable_get_resource_class(shaped_ssavar->ssavar->variable);
		xpoint->base.count = 0;
		xpoint->shaped_region = self->shaped_region;
		jive_region_varcut_register_xpoint(self, xpoint);
	} else {
		xpoint = reinterpret_cast<jive_regvar_xpoint *>(i.ptr());
	}
	
	size_t old_count = xpoint->base.count;
	xpoint->base.count += count;
	return old_count;
}

size_t
jive_region_varcut_ssavar_remove(
	jive_region_varcut * self,
	jive_shaped_ssavar * shaped_ssavar,
	size_t count)
{
	auto i = self->base.ssavar_map.find(shaped_ssavar);
	
	if (i == self->base.ssavar_map.end()) {
		return 0;
	}
	jive_regvar_xpoint * xpoint = reinterpret_cast<jive_regvar_xpoint *>(i.ptr());
	
	size_t old_count = xpoint->base.count;
	xpoint->base.count -= count;
	if (xpoint->base.count == 0) {
		jive_region_varcut_unregister_xpoint(self, xpoint);
		delete xpoint;
	}
	
	return old_count;
}

void
jive_region_varcut_ssavar_divert_origin(jive_region_varcut * self,
	jive_shaped_ssavar * shaped_ssavar, jive::output * origin)
{
	jive_mutable_varcut_ssavar_divert_origin(&self->base, shaped_ssavar, origin);
}

void
jive_region_varcut_ssavar_variable_change(
	jive_region_varcut * self,
	jive_shaped_ssavar * shaped_ssavar,
	jive_variable * variable)
{
	jive_mutable_varcut_ssavar_variable_change(&self->base, shaped_ssavar, variable);
}

void
jive_region_varcut_ssavar_rescls_change(
	jive_region_varcut * self,
	jive_shaped_ssavar * shaped_ssavar,
	const jive_resource_class * rescls)
{
	jive_mutable_varcut_ssavar_rescls_change(&self->base, shaped_ssavar, rescls);
}
