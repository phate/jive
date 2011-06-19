#include <jive/regalloc/xpoint-private.h>

#include <jive/common.h>

#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/shaped-node.h>
#include <jive/regalloc/shaped-variable.h>
#include <jive/vsdg/basetype.h>
#include <jive/vsdg/resource-private.h>
#include <jive/vsdg/variable.h>

jive_nodevar_xpoint *
jive_nodevar_xpoint_create(jive_shaped_node * shaped_node, jive_shaped_ssavar * shaped_ssavar)
{
	jive_context * context = shaped_node->shaped_graph->context;
	
	jive_nodevar_xpoint * xpoint = jive_context_malloc(context, sizeof(*xpoint));
	xpoint->shaped_node = shaped_node;
	xpoint->shaped_ssavar = shaped_ssavar;
	xpoint->before_count = 0;
	xpoint->cross_count = 0;
	xpoint->after_count = 0;
	
	jive_nodevar_xpoint_hash_byssavar_insert(&shaped_node->ssavar_xpoints, xpoint);
	jive_nodevar_xpoint_hash_bynode_insert(&shaped_ssavar->node_xpoints, xpoint);
	
	return xpoint;
}

void
jive_nodevar_xpoint_destroy(jive_nodevar_xpoint * xpoint)
{
	jive_shaped_node * shaped_node = xpoint->shaped_node;
	jive_shaped_ssavar * shaped_ssavar = xpoint->shaped_ssavar;
	jive_context * context = shaped_node->shaped_graph->context;
	
	jive_nodevar_xpoint_hash_byssavar_remove(&shaped_node->ssavar_xpoints, xpoint);
	jive_nodevar_xpoint_hash_bynode_remove(&shaped_ssavar->node_xpoints, xpoint);
	
	jive_context_free(context, xpoint);
}

void
jive_varcut_init(jive_varcut * self, jive_context * context)
{
	jive_cutvar_xpoint_hash_byssavar_init(&self->ssavar_map, context);
	jive_cutvar_xpoint_hash_byorigin_init(&self->origin_map, context);
	jive_cutvar_xpoint_hash_byvariable_init(&self->variable_map, context);
	self->context = context;
	jive_resource_class_count_init(&self->use_counts);
	self->xpoints.first = self->xpoints.last = 0;
}

void
jive_varcut_fini(jive_varcut * self)
{
	jive_mutable_varcut_clear(self);
	jive_resource_class_count_fini(&self->use_counts, self->context);
	jive_cutvar_xpoint_hash_byssavar_fini(&self->ssavar_map);
	jive_cutvar_xpoint_hash_byorigin_fini(&self->origin_map);
	jive_cutvar_xpoint_hash_byvariable_fini(&self->variable_map);
}

jive_shaped_ssavar *
jive_varcut_map_output(const jive_varcut * self, jive_output * output)
{
	jive_cutvar_xpoint * xpoint = jive_cutvar_xpoint_hash_byorigin_lookup(&self->origin_map, output);
	if (xpoint)
		return xpoint->shaped_ssavar;
	else
		return 0;
}

jive_shaped_ssavar *
jive_varcut_map_variable(const jive_varcut * self, jive_variable * variable)
{
	jive_cutvar_xpoint * xpoint = jive_cutvar_xpoint_hash_byvariable_lookup(&self->variable_map, variable);
	if (xpoint)
		return xpoint->shaped_ssavar;
	else
		return 0;
}

static void
jive_mutable_varcut_register_xpoint(jive_mutable_varcut * self, jive_cutvar_xpoint * xpoint)
{
	jive_cutvar_xpoint_hash_byssavar_insert(&self->ssavar_map, xpoint);
	jive_cutvar_xpoint_hash_byorigin_insert(&self->origin_map, xpoint);
	jive_cutvar_xpoint_hash_byvariable_insert(&self->variable_map, xpoint);
	JIVE_LIST_PUSH_BACK(self->xpoints, xpoint, varcut_xpoints_list);
	jive_resource_class_count_add(&self->use_counts, self->context, xpoint->rescls);
}

static void
jive_mutable_varcut_unregister_xpoint(jive_mutable_varcut * self, jive_cutvar_xpoint * xpoint)
{
	jive_cutvar_xpoint_hash_byssavar_remove(&self->ssavar_map, xpoint);
	jive_cutvar_xpoint_hash_byorigin_remove(&self->origin_map, xpoint);
	jive_cutvar_xpoint_hash_byvariable_remove(&self->variable_map, xpoint);
	JIVE_LIST_REMOVE(self->xpoints, xpoint, varcut_xpoints_list);
	jive_resource_class_count_sub(&self->use_counts, self->context, xpoint->rescls);
}

void
jive_mutable_varcut_clear(jive_mutable_varcut * self)
{
	jive_resource_class_count_clear(&self->use_counts, self->context);
	
	jive_cutvar_xpoint * xpoint, * next_xpoint;
	JIVE_LIST_ITERATE_SAFE(self->xpoints, xpoint, next_xpoint, varcut_xpoints_list) {
		jive_cutvar_xpoint_hash_byssavar_remove(&self->ssavar_map, xpoint);
		jive_cutvar_xpoint_hash_byorigin_remove(&self->origin_map, xpoint);
		jive_cutvar_xpoint_hash_byvariable_remove(&self->variable_map, xpoint);
		JIVE_LIST_REMOVE(self->xpoints, xpoint, varcut_xpoints_list);
		
		jive_context_free(self->context, xpoint);
	}
}

void
jive_mutable_varcut_assign(jive_mutable_varcut * self, const jive_varcut * other)
{
	jive_mutable_varcut_clear(self);
	
	const jive_cutvar_xpoint * src_xpoint;
	JIVE_LIST_ITERATE(other->xpoints, src_xpoint, varcut_xpoints_list) {
		jive_cutvar_xpoint * xpoint = jive_context_malloc(self->context, sizeof(*xpoint));
		xpoint->shaped_ssavar = src_xpoint->shaped_ssavar;
		xpoint->origin = src_xpoint->origin;
		xpoint->variable = src_xpoint->variable;
		xpoint->rescls = src_xpoint->rescls;
		xpoint->count = src_xpoint->count;
		jive_mutable_varcut_register_xpoint(self, xpoint);
	}
}

size_t
jive_mutable_varcut_ssavar_add(jive_mutable_varcut * self, jive_shaped_ssavar * shaped_ssavar, size_t count)
{
	if (!count)
		return 0;
	
	jive_cutvar_xpoint * xpoint = jive_cutvar_xpoint_hash_byssavar_lookup(&self->ssavar_map, shaped_ssavar);
	
	if (!xpoint) {
		xpoint = jive_context_malloc(self->context, sizeof(*xpoint));
		xpoint->shaped_ssavar = shaped_ssavar;
		xpoint->origin = shaped_ssavar->ssavar->origin;
		xpoint->variable = shaped_ssavar->ssavar->variable;
		xpoint->rescls = jive_variable_get_resource_class(shaped_ssavar->ssavar->variable);
		xpoint->count = 0;
		jive_mutable_varcut_register_xpoint(self, xpoint);
	}
	
	size_t old_count = xpoint->count;
	xpoint->count += count;
	return old_count;
}

size_t
jive_mutable_varcut_ssavar_remove(jive_mutable_varcut * self, jive_shaped_ssavar * shaped_ssavar, size_t count)
{
	jive_cutvar_xpoint * xpoint = jive_cutvar_xpoint_hash_byssavar_lookup(&self->ssavar_map, shaped_ssavar);
	
	if (!xpoint)
		return 0;
	
	size_t old_count = xpoint->count;
	xpoint->count -= count;
	if (xpoint->count == 0) {
		jive_mutable_varcut_unregister_xpoint(self, xpoint);
		jive_context_free(self->context, xpoint);
	}
	
	return old_count;
}

void
jive_mutable_varcut_ssavar_remove_full(jive_mutable_varcut * self, jive_shaped_ssavar * shaped_ssavar)
{
	jive_cutvar_xpoint * xpoint = jive_cutvar_xpoint_hash_byssavar_lookup(&self->ssavar_map, shaped_ssavar);
	
	if (!xpoint)
		return;
	
	jive_mutable_varcut_unregister_xpoint(self, xpoint);
	jive_context_free(self->context, xpoint);
}

void
jive_mutable_varcut_ssavar_divert_origin(jive_mutable_varcut * self, jive_shaped_ssavar * shaped_ssavar, jive_output * origin)
{
	jive_cutvar_xpoint * xpoint = jive_cutvar_xpoint_hash_byssavar_lookup(&self->ssavar_map, shaped_ssavar);
	
	if (!xpoint)
		return;
	
	jive_cutvar_xpoint_hash_byorigin_remove(&self->origin_map, xpoint);
	xpoint->origin = origin;
	jive_cutvar_xpoint_hash_byorigin_insert(&self->origin_map, xpoint);
}

void
jive_mutable_varcut_ssavar_variable_change(jive_mutable_varcut * self, jive_shaped_ssavar * shaped_ssavar, jive_variable * variable)
{
	jive_cutvar_xpoint * xpoint = jive_cutvar_xpoint_hash_byssavar_lookup(&self->ssavar_map, shaped_ssavar);
	
	if (!xpoint)
		return;
	
	jive_cutvar_xpoint_hash_byvariable_remove(&self->variable_map, xpoint);
	xpoint->variable = variable;
	jive_cutvar_xpoint_hash_byvariable_insert(&self->variable_map, xpoint);
}


void
jive_mutable_varcut_ssavar_rescls_change(jive_mutable_varcut * self, jive_shaped_ssavar * shaped_ssavar, const jive_resource_class * rescls)
{
	jive_cutvar_xpoint * xpoint = jive_cutvar_xpoint_hash_byssavar_lookup(&self->ssavar_map, shaped_ssavar);
	
	if (!xpoint)
		return;
	
	jive_resource_class_count_change(&self->use_counts, self->context, xpoint->rescls, rescls);
	xpoint->rescls = rescls;
}


static void
jive_region_varcut_register_xpoint(jive_region_varcut * self, jive_regvar_xpoint * xpoint)
{
	jive_cutvar_xpoint_hash_byssavar_insert(&self->base.ssavar_map, &xpoint->base);
	jive_cutvar_xpoint_hash_byorigin_insert(&self->base.origin_map, &xpoint->base);
	jive_cutvar_xpoint_hash_byvariable_insert(&self->base.variable_map, &xpoint->base);
	jive_regvar_xpoint_hash_byregion_insert(&xpoint->base.shaped_ssavar->region_xpoints, xpoint);
	JIVE_LIST_PUSH_BACK(self->base.xpoints, &xpoint->base, varcut_xpoints_list);
	jive_resource_class_count_add(&self->base.use_counts, self->base.context, xpoint->base.rescls);
}

static void
jive_region_varcut_unregister_xpoint(jive_region_varcut * self, jive_regvar_xpoint * xpoint)
{
	jive_cutvar_xpoint_hash_byssavar_remove(&self->base.ssavar_map, &xpoint->base);
	jive_cutvar_xpoint_hash_byorigin_remove(&self->base.origin_map, &xpoint->base);
	jive_cutvar_xpoint_hash_byvariable_remove(&self->base.variable_map, &xpoint->base);
	jive_regvar_xpoint_hash_byregion_remove(&xpoint->base.shaped_ssavar->region_xpoints, xpoint);
	JIVE_LIST_REMOVE(self->base.xpoints, &xpoint->base, varcut_xpoints_list);
	jive_resource_class_count_add(&self->base.use_counts, self->base.context, xpoint->base.rescls);
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
	
	jive_resource_class_count_fini(&self->base.use_counts, self->base.context);
	jive_cutvar_xpoint_hash_byssavar_fini(&self->base.ssavar_map);
	jive_cutvar_xpoint_hash_byorigin_fini(&self->base.origin_map);
	jive_cutvar_xpoint_hash_byvariable_fini(&self->base.variable_map);
}

size_t
jive_region_varcut_ssavar_add(jive_region_varcut * self, jive_shaped_ssavar * shaped_ssavar, size_t count)
{
	if (!count)
		return 0;
	
	jive_regvar_xpoint * xpoint;
	xpoint = (jive_regvar_xpoint *) jive_cutvar_xpoint_hash_byssavar_lookup(&self->base.ssavar_map, shaped_ssavar);
	
	if (!xpoint) {
		xpoint = jive_context_malloc(self->base.context, sizeof(*xpoint));
		xpoint->base.shaped_ssavar = shaped_ssavar;
		xpoint->base.origin = shaped_ssavar->ssavar->origin;
		xpoint->base.variable = shaped_ssavar->ssavar->variable;
		xpoint->base.rescls = jive_variable_get_resource_class(shaped_ssavar->ssavar->variable);
		xpoint->base.count = 0;
		xpoint->shaped_region = self->shaped_region;
		jive_region_varcut_register_xpoint(self, xpoint);
	}
	
	size_t old_count = xpoint->base.count;
	xpoint->base.count += count;
	return old_count;
}

size_t
jive_region_varcut_ssavar_remove(jive_region_varcut * self, jive_shaped_ssavar * shaped_ssavar, size_t count)
{
	jive_regvar_xpoint * xpoint;
	xpoint = (jive_regvar_xpoint *) jive_cutvar_xpoint_hash_byssavar_lookup(&self->base.ssavar_map, shaped_ssavar);
	
	if (!xpoint)
		return 0;
	
	size_t old_count = xpoint->base.count;
	xpoint->base.count -= count;
	if (xpoint->base.count == 0) {
		jive_region_varcut_unregister_xpoint(self, xpoint);
		jive_context_free(self->base.context, xpoint);
	}
	
	return old_count;
}

void
jive_region_varcut_ssavar_divert_origin(jive_region_varcut * self, jive_shaped_ssavar * shaped_ssavar, jive_output * origin)
{
	jive_mutable_varcut_ssavar_divert_origin(&self->base, shaped_ssavar, origin);
}

void
jive_region_varcut_ssavar_variable_change(jive_region_varcut * self, jive_shaped_ssavar * shaped_ssavar, jive_variable * variable)
{
	jive_mutable_varcut_ssavar_variable_change(&self->base, shaped_ssavar, variable);
}

void
jive_region_varcut_ssavar_rescls_change(jive_region_varcut * self, jive_shaped_ssavar * shaped_ssavar, const jive_resource_class * rescls)
{
	jive_mutable_varcut_ssavar_rescls_change(&self->base, shaped_ssavar, rescls);
}
