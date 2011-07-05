#ifndef JIVE_REGALLOC_SHAPED_NODE_PRIVATE_H
#define JIVE_REGALLOC_SHAPED_NODE_PRIVATE_H

#include <jive/common.h>

#include <jive/vsdg/variable.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/shaped-node.h>
#include <jive/regalloc/shaped-variable.h>
#include <jive/regalloc/shaped-variable-private.h>
#include <jive/regalloc/xpoint-private.h>
#include <jive/vsdg/resource-private.h>

jive_shaped_node *
jive_shaped_node_create(struct jive_cut * cut, struct jive_node * node);

static inline jive_nodevar_xpoint *
jive_shaped_node_get_ssavar_xpoint(jive_shaped_node * self, jive_shaped_ssavar * shaped_ssavar)
{
	jive_nodevar_xpoint * xpoint = jive_nodevar_xpoint_hash_byssavar_lookup(&self->ssavar_xpoints, shaped_ssavar);
	if (!xpoint)
		xpoint = jive_nodevar_xpoint_create(self, shaped_ssavar);
	return xpoint;
}

static inline void
jive_shaped_node_inc_active_after(jive_shaped_node * self, jive_nodevar_xpoint * xpoint,
	jive_shaped_ssavar * shaped_ssavar, struct jive_variable * variable, size_t count)
{
	if (xpoint->after_count == 0) {
		struct jive_nodevar_xpoint_hash_byssavar_iterator i;
		JIVE_HASH_ITERATE(jive_nodevar_xpoint_hash_byssavar, self->ssavar_xpoints, i) {
			jive_nodevar_xpoint * other_xpoint = i.entry;
			if (!other_xpoint->after_count) continue;
			if (other_xpoint == xpoint) continue;
			jive_variable_interference_add(
				jive_shaped_graph_map_variable(self->shaped_graph, variable),
				jive_shaped_graph_map_variable(self->shaped_graph, other_xpoint->shaped_ssavar->ssavar->variable)
			);
		}
		const jive_resource_class * overflow;
		overflow = jive_resource_class_count_add(&self->use_count_after, jive_variable_get_resource_class(variable));
		JIVE_DEBUG_ASSERT(!overflow);
	}
	xpoint->after_count += count;
}

static inline void
jive_shaped_node_dec_active_after(jive_shaped_node * self, jive_nodevar_xpoint * xpoint,
	jive_shaped_ssavar * shaped_ssavar, struct jive_variable * variable, size_t count)
{
	JIVE_DEBUG_ASSERT(xpoint->after_count >= count);
	xpoint->after_count -= count;
	if (xpoint->after_count == 0) {
		struct jive_nodevar_xpoint_hash_byssavar_iterator i;
		JIVE_HASH_ITERATE(jive_nodevar_xpoint_hash_byssavar, self->ssavar_xpoints, i) {
			jive_nodevar_xpoint * other_xpoint = i.entry;
			if (!other_xpoint->after_count) continue;
			if (other_xpoint == xpoint) continue;
			jive_variable_interference_remove(
				jive_shaped_graph_map_variable(self->shaped_graph, variable),
				jive_shaped_graph_map_variable(self->shaped_graph, other_xpoint->shaped_ssavar->ssavar->variable)
			);
		}
		jive_resource_class_count_sub(&self->use_count_after, jive_variable_get_resource_class(variable));
	}
}

static inline void
jive_shaped_node_inc_active_before(jive_shaped_node * self, jive_nodevar_xpoint * xpoint,
	jive_shaped_ssavar * shaped_ssavar, struct jive_variable * variable, size_t count)
{
	if (xpoint->before_count == 0) {
		struct jive_nodevar_xpoint_hash_byssavar_iterator i;
		JIVE_HASH_ITERATE(jive_nodevar_xpoint_hash_byssavar, self->ssavar_xpoints, i) {
			jive_nodevar_xpoint * other_xpoint = i.entry;
			if (!other_xpoint->before_count) continue;
			if (other_xpoint == xpoint) continue;
			jive_variable_interference_add(
				jive_shaped_graph_map_variable(self->shaped_graph, variable),
				jive_shaped_graph_map_variable(self->shaped_graph, other_xpoint->shaped_ssavar->ssavar->variable)
			);
		}
		const jive_resource_class * overflow;
		overflow = jive_resource_class_count_add(&self->use_count_before, jive_variable_get_resource_class(variable));
		(void) overflow;
		JIVE_DEBUG_ASSERT(!overflow);
	}
	xpoint->before_count += count;
}

static inline void
jive_shaped_node_dec_active_before(jive_shaped_node * self, jive_nodevar_xpoint * xpoint,
	jive_shaped_ssavar * shaped_ssavar, struct jive_variable * variable, size_t count)
{
	JIVE_DEBUG_ASSERT(xpoint->before_count >= count);
	xpoint->before_count -= count;
	if (xpoint->before_count == 0) {
		struct jive_nodevar_xpoint_hash_byssavar_iterator i;
		JIVE_HASH_ITERATE(jive_nodevar_xpoint_hash_byssavar, self->ssavar_xpoints, i) {
			jive_nodevar_xpoint * other_xpoint = i.entry;
			if (!other_xpoint->before_count) continue;
			if (other_xpoint == xpoint) continue;
			jive_variable_interference_remove(
				jive_shaped_graph_map_variable(self->shaped_graph, variable),
				jive_shaped_graph_map_variable(self->shaped_graph, other_xpoint->shaped_ssavar->ssavar->variable)
			);
		}
		jive_resource_class_count_sub(&self->use_count_before, jive_variable_get_resource_class(variable));
	}
}

static inline void
jive_shaped_node_add_ssavar_before(jive_shaped_node * self, jive_shaped_ssavar * shaped_ssavar, struct jive_variable * variable, size_t count)
{
	if (count == 0) return;
	jive_nodevar_xpoint * xpoint = jive_shaped_node_get_ssavar_xpoint(self, shaped_ssavar);
	
	jive_shaped_node_inc_active_before(self, xpoint, shaped_ssavar, variable, count);
}

static inline void
jive_shaped_node_remove_ssavar_before(jive_shaped_node * self, jive_shaped_ssavar * shaped_ssavar, struct jive_variable * variable, size_t count)
{
	if (count == 0) return;
	jive_nodevar_xpoint * xpoint = jive_nodevar_xpoint_hash_byssavar_lookup(&self->ssavar_xpoints, shaped_ssavar);
	JIVE_DEBUG_ASSERT(xpoint);
	
	jive_shaped_node_dec_active_before(self, xpoint, shaped_ssavar, variable, count);
	jive_nodevar_xpoint_put(xpoint);
}

static inline void
jive_shaped_node_add_ssavar_crossed(jive_shaped_node * self, jive_shaped_ssavar * shaped_ssavar, struct jive_variable * variable, size_t count)
{
	if (count == 0) return;
	jive_nodevar_xpoint * xpoint = jive_shaped_node_get_ssavar_xpoint(self, shaped_ssavar);
	
	xpoint->cross_count += count;
	
	jive_shaped_node_inc_active_before(self, xpoint, shaped_ssavar, variable, count);
	jive_shaped_node_inc_active_after(self, xpoint, shaped_ssavar, variable, count);
}

static inline void
jive_shaped_node_remove_ssavar_crossed(jive_shaped_node * self, jive_shaped_ssavar * shaped_ssavar, struct jive_variable * variable, size_t count)
{
	if (count == 0) return;
	jive_nodevar_xpoint * xpoint = jive_nodevar_xpoint_hash_byssavar_lookup(&self->ssavar_xpoints, shaped_ssavar);
	JIVE_DEBUG_ASSERT(xpoint);
	
	xpoint->cross_count -= count;
	
	jive_shaped_node_dec_active_before(self, xpoint, shaped_ssavar, variable, count);
	jive_shaped_node_dec_active_after(self, xpoint, shaped_ssavar, variable, count);
	jive_nodevar_xpoint_put(xpoint);
}

static inline void
jive_shaped_node_add_ssavar_after(jive_shaped_node * self, jive_shaped_ssavar * shaped_ssavar, struct jive_variable * variable, size_t count)
{
	if (count == 0) return;
	jive_nodevar_xpoint * xpoint = jive_shaped_node_get_ssavar_xpoint(self, shaped_ssavar);
	
	jive_shaped_node_inc_active_after(self, xpoint, shaped_ssavar, variable, count);
}

static inline void
jive_shaped_node_remove_ssavar_after(jive_shaped_node * self, jive_shaped_ssavar * shaped_ssavar, struct jive_variable * variable, size_t count)
{
	if (count == 0) return;
	jive_nodevar_xpoint * xpoint = jive_nodevar_xpoint_hash_byssavar_lookup(&self->ssavar_xpoints, shaped_ssavar);
	JIVE_DEBUG_ASSERT(xpoint);
	
	jive_shaped_node_dec_active_after(self, xpoint, shaped_ssavar, variable, count);
	jive_nodevar_xpoint_put(xpoint);
}

static inline bool
jive_shaped_node_is_resource_name_active_after(const jive_shaped_node * self, const struct jive_resource_name * name)
{
	struct jive_nodevar_xpoint_hash_byssavar_iterator i;
	JIVE_HASH_ITERATE(jive_nodevar_xpoint_hash_byssavar, self->ssavar_xpoints, i) {
		jive_nodevar_xpoint * xpoint = i.entry;
		if (!xpoint->after_count) continue;
		if (jive_variable_get_resource_name(xpoint->shaped_ssavar->ssavar->variable) == name)
			return true;
	}
	
	return false;
}

static inline bool
jive_shaped_node_is_resource_name_active_before(const jive_shaped_node * self, const struct jive_resource_name * name)
{
	struct jive_nodevar_xpoint_hash_byssavar_iterator i;
	JIVE_HASH_ITERATE(jive_nodevar_xpoint_hash_byssavar, self->ssavar_xpoints, i) {
		jive_nodevar_xpoint * xpoint = i.entry;
		if (!xpoint->before_count) continue;
		if (jive_variable_get_resource_name(xpoint->shaped_ssavar->ssavar->variable) == name)
			return true;
	}
	
	return false;
}

#endif
