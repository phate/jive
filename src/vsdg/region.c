/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/region.h>

#include <jive/common.h>

#include <jive/util/list.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/graph-private.h>
#include <jive/vsdg/region-ssavar-use-private.h>
#include <jive/vsdg/substitution.h>
#include <jive/vsdg/traverser.h>
#include <jive/vsdg/variable.h>

static inline void
jive_region_attrs_init(jive_region_attrs * attrs)
{
	attrs->align = 1;
	attrs->section = jive_region_section_inherit;
	attrs->is_looped = false;
	attrs->is_floating = false;
}

void
jive_region_init_(jive_region * self, jive_graph * graph, jive_region * parent)
{
	self->graph = graph;
	self->parent = parent;
	self->stackframe = 0;
	
	self->nodes.first = self->nodes.last = 0;
	self->top_nodes.first = self->top_nodes.last = 0;
	self->subregions.first = self->subregions.last = 0;
	self->hull.first = self->hull.last = 0;
	self->region_subregions_list.prev = self->region_subregions_list.next = 0;
	self->top = self->bottom = 0;
	jive_region_attrs_init(&self->attrs);
	jive_region_ssavar_hash_init(&self->used_ssavars, graph->context);
	
	if (parent) {
		JIVE_LIST_PUSH_BACK(parent->subregions, self, region_subregions_list);
		self->depth = parent->depth + 1;
	} else self->depth = 0;
	
	self->anchor = 0;
	
	jive_graph_notify_region_create(graph, self);
}

void
jive_region_fini_(jive_region * self)
{
	JIVE_DEBUG_ASSERT(jive_region_empty(self));
	JIVE_DEBUG_ASSERT(self->nodes.first == 0 && self->nodes.last == 0);
	JIVE_DEBUG_ASSERT(self->subregions.first == 0 && self->subregions.last == 0);
	
	jive_graph_notify_region_destroy(self->graph, self);
	
	jive_region_ssavar_hash_fini(&self->used_ssavars);
	if (self->parent)
		JIVE_LIST_REMOVE(self->parent->subregions, self, region_subregions_list);
	/* FIXME: destroy stackframe! */
	/* if (self->stackframe)
		jive_stackframe_destroy(self->stackframe); */
}

bool
jive_region_depends_on_region(const jive_region * self, const jive_region * region)
{
	jive_node * node;
	JIVE_LIST_ITERATE(self->nodes, node, region_nodes_list) {
		if (jive_node_depends_on_region(node, region)) return true;
	}
	return false;
}

static void
jive_region_set_depth_recursive(jive_region * self, size_t new_depth)
{
	self->depth = new_depth;

	jive_region * subregion;
	JIVE_LIST_ITERATE(self->subregions, subregion, region_subregions_list) {
		jive_region_set_depth_recursive(subregion, new_depth + 1);
	}
}

void
jive_region_reparent(jive_region * self, jive_region * new_parent)
{
	JIVE_LIST_REMOVE(self->parent->subregions, self, region_subregions_list);

	jive_region_hull_remove_hull_from_parents(self);

	self->parent = new_parent;
	size_t new_depth = new_parent->depth + 1;
	if (new_depth != self->depth)
		jive_region_set_depth_recursive(self, new_depth);

	jive_region_hull_add_hull_to_parents(self);

	JIVE_LIST_PUSH_BACK(self->parent->subregions, self, region_subregions_list);
}

void
jive_region_destroy(jive_region * self)
{
	jive_region_fini_(self);
	jive_context_free(self->graph->context, self);
}

void
jive_region_prune_subregions_(jive_region * self)
{
	jive_region * subregion;
	subregion = self->subregions.first;
	while(subregion) {
		jive_region * next = subregion->region_subregions_list.next;
		if (jive_region_empty(subregion)) {
			JIVE_LIST_REMOVE(self->subregions, subregion, region_subregions_list);
			jive_region_destroy(subregion);
		}
		subregion = next;
	}
}

jive_region *
jive_region_create_subregion(jive_region * self)
{
	jive_region * subregion = jive_context_malloc(self->graph->context, sizeof(*subregion));
	jive_region_init_(subregion, self->graph, self);
	return subregion;
}

jive_floating_region
jive_floating_region_create(jive_graph * graph)
{
	jive_floating_region floating_region;
	floating_region.region = jive_context_malloc(graph->context, sizeof(*floating_region.region));
	jive_region_init_(floating_region.region, graph, graph->root_region);
	floating_region.region ->attrs.is_floating = true;
	graph->floating_region_count ++;
	return floating_region;
}

void
jive_floating_region_settle(jive_floating_region floating_region)
{
	JIVE_DEBUG_ASSERT(floating_region.region->attrs.is_floating);
	floating_region.region->attrs.is_floating = false;
	floating_region.region->graph->floating_region_count --;
}

void
jive_region_add_used_ssavar(jive_region * self, jive_ssavar * ssavar)
{
	if (ssavar->origin->node()->region->depth >= self->depth) return;
	if (self->attrs.is_looped) {
		jive_region_ssavar_use * use;
		use = jive_region_ssavar_hash_lookup(&self->used_ssavars, ssavar);
		if (use)
			use->count ++;
		else {
			use = jive_context_malloc(self->graph->context, sizeof(*use));
			use->region = self;
			use->ssavar = ssavar;
			use->count = 1;
			jive_region_ssavar_hash_insert(&self->used_ssavars, use);
			jive_ssavar_region_hash_insert(&ssavar->assigned_regions, use);
			
			jive_graph_notify_region_add_used_ssavar(self->graph, self, ssavar);
		}
	}
	
	jive_region_add_used_ssavar(self->parent, ssavar);
}

void
jive_region_remove_used_ssavar(jive_region * self, jive_ssavar * ssavar)
{
	if (ssavar->origin->node()->region->depth >= self->depth) return;
	if (self->attrs.is_looped) {
		jive_region_ssavar_use * use;
		use = jive_region_ssavar_hash_lookup(&self->used_ssavars, ssavar);
		use->count --;
		if (use->count == 0) {
			jive_region_ssavar_hash_remove(&self->used_ssavars, use);
			jive_ssavar_region_hash_remove(&ssavar->assigned_regions, use);
			
			jive_graph_notify_region_remove_used_ssavar(self->graph, self, ssavar);
			jive_context_free(self->graph->context, use);
		}
	}
	
	jive_region_remove_used_ssavar(self->parent, ssavar);
}

typedef struct jive_level_nodes jive_level_nodes;
struct jive_level_nodes {
	jive_node ** items;
	size_t nitems, space;
};

typedef struct jive_copy_context jive_copy_context;
struct jive_copy_context {
	jive_level_nodes * depths;
	size_t max_depth_plus_one, space;
};

static void
jive_copy_context_init(jive_copy_context * self)
{
	self->depths = 0;
	self->max_depth_plus_one = 0;
	self->space = 0;
}

static void
jive_copy_context_fini(jive_copy_context * self, jive_context * context)
{
	size_t n;
	for (n = 0; n < self->max_depth_plus_one; n++)
		jive_context_free(context, self->depths[n].items);
	jive_context_free(context, self->depths);
}

static void
jive_level_nodes_append(jive_level_nodes * level, jive_context * context, jive_node * node)
{
	if (level->nitems == level->space) {
		level->space =  level->space * 2 + 1;
		level->items = jive_context_realloc(context, level->items, level->space * sizeof(jive_node *));
	}
	level->items[level->nitems ++] = node;
}

static void
jive_copy_context_append(jive_copy_context * self, jive_context * context, jive_node * node)
{
	if (node->depth_from_root >= self->space) {
		size_t new_space = self->space * 2;
		if (new_space <= node->depth_from_root)
			new_space = node->depth_from_root + 1;
		self->depths = jive_context_realloc(context, self->depths, new_space * sizeof(self->depths[0]));
		size_t n;
		for (n = self->space; n < new_space; n++) {
			self->depths[n].items = 0;
			self->depths[n].space = 0;
			self->depths[n].nitems = 0;
		}
		self->space = new_space;
	}
	jive_level_nodes_append(&self->depths[node->depth_from_root], context, node);
	if (node->depth_from_root + 1 > self->max_depth_plus_one)
		self->max_depth_plus_one = node->depth_from_root + 1;
}

static void
pre_copy_region(jive_region * target_region, const jive_region * original_region,
	jive_copy_context * copy_context, jive_context * context, jive_substitution_map * substitution,
	bool copy_top, bool copy_bottom)
{
	jive_node * node;
	JIVE_LIST_ITERATE(original_region->nodes, node, region_nodes_list) {
		if (!copy_top && node == original_region->top) continue;
		if (!copy_bottom && node == original_region->bottom) continue;
		jive_copy_context_append(copy_context, context, node);
	}
	
	jive_region * subregion;
	JIVE_LIST_ITERATE(original_region->subregions, subregion, region_subregions_list) {
		jive_region * target_subregion = jive_region_create_subregion(target_region);
		target_subregion->attrs = subregion->attrs;
		jive_substitution_map_add_region(substitution, subregion, target_subregion);
		pre_copy_region(target_subregion, subregion, copy_context, context, substitution, true, true);
	}
}

void
jive_region_copy_substitute(const jive_region * self, jive_region * target,
	jive_substitution_map * substitution,
	bool copy_top, bool copy_bottom)
{
	jive_context * context = target->graph->context;
	jive_copy_context copy_context;
	jive_copy_context_init(&copy_context);
	
	jive_substitution_map_add_region(substitution, self, target);
	pre_copy_region(target, self, &copy_context, context, substitution, copy_top, copy_bottom);
	
	size_t depth;
	for(depth = 0; depth < copy_context.max_depth_plus_one; depth ++) {
		size_t n;
		for(n = 0; n < copy_context.depths[depth].nitems; n++) {
			jive_node * node = copy_context.depths[depth].items[n];
			jive_region * target_subregion = jive_substitution_map_lookup_region(substitution, node->region);
			jive_node * new_node = jive_node_copy_substitute(node, target_subregion, substitution);
			if (node->region->top == node)
				target_subregion->top = new_node;
			if (node->region->bottom == node)
				target_subregion->bottom = new_node;
		}
	}
	
	jive_copy_context_fini(&copy_context, context);
}

void
jive_region_check_move_floating(jive_region * self, jive_region * edge_origin)
{
	jive_region * movable = self;
	jive_region * parent = self->parent;
	/* find closest common parent, as well as the region directly below it
	on the path */
	while (movable->depth > edge_origin->depth) {
		movable = parent;
		parent = parent->parent;
	}
	/* if the search ends up in the origin region of the new edge, then we
	know that the edge to be inserted points from one outer region into
	an inner region, and is thus valid */
	if (movable == edge_origin)
		return;
	/* if the search ends up somewhere else, then we have found the
	region which we must reparent to make this a valid edge */
	if (!movable->attrs.is_floating)
		jive_context_fatal_error(self->graph->context, "Invalid edge");
	jive_region_reparent(movable, edge_origin);
}

static jive_region_hull_entry *
jive_region_hull_entry_create(jive_region * region, jive_input * input)
{
	jive_context * context = input->node->region->graph->context;

	jive_region_hull_entry * entry = jive_context_malloc(context, sizeof(*entry));
	entry->region_hull_list.prev = entry->region_hull_list.next = NULL;
	entry->input_hull_list.prev = entry->input_hull_list.next = NULL;
	entry->input = input;
	entry->region = region;

	JIVE_LIST_PUSH_BACK(region->hull, entry, region_hull_list);
	JIVE_LIST_PUSH_BACK(input->hull, entry, input_hull_list);
	return entry;
}

static void
jive_region_hull_entry_destroy(jive_region_hull_entry * entry)
{
	JIVE_LIST_REMOVE(entry->input->hull, entry, input_hull_list);
	JIVE_LIST_REMOVE(entry->region->hull, entry, region_hull_list);
	jive_context_free(entry->input->node->region->graph->context, entry);
}

void
jive_region_hull_add_input(jive_region * region, jive_input * input)
{
	jive_region * origin_region = input->producer()->region;
	while (region->depth > origin_region->depth) {
		jive_region_hull_entry_create(region, input);
		region = region->parent;
	}
}

void
jive_region_hull_remove_input(struct jive_region * region, struct jive_input * input)
{
	jive_region_hull_entry * entry, * next_entry;
	JIVE_LIST_ITERATE_SAFE(input->hull, entry, next_entry, input_hull_list) {
		if (region->depth >= entry->region->depth) {
			jive_region_hull_entry_destroy(entry);
		}
	}
}

#ifdef JIVE_DEBUG
void
jive_region_verify_hull(struct jive_region * region)
{
	jive_region * subregion;
	JIVE_LIST_ITERATE(region->subregions, subregion, region_subregions_list)
		jive_region_verify_hull(subregion);

	/* check whether all inputs of the nodes in the region are in the hull */
	jive_node * node;
	JIVE_LIST_ITERATE(region->nodes, node, region_nodes_list) {
		size_t i;
		for (i = 0; i < node->ninputs; i++) {
			if (node->producer(i)->region->depth < region->depth) {
				jive_region_hull_entry * entry;
				JIVE_LIST_ITERATE(region->hull, entry, region_hull_list) {
					if (entry->input == node->inputs[i])
						break;
				}
				if (entry == NULL)
					JIVE_DEBUG_ASSERT(0);
			}
		}
	}

	/* check whether all inputs from the subregion hulls that originate from a node in a parent region
	are in the hull */
	JIVE_LIST_ITERATE(region->subregions, subregion, region_subregions_list) {
		jive_region_hull_entry * sub_entry;
		JIVE_LIST_ITERATE(subregion->hull, sub_entry, region_hull_list) {
			if (sub_entry->input->origin()->node()->region->depth < region->depth) {
				jive_region_hull_entry * entry;
				JIVE_LIST_ITERATE(region->hull, entry, region_hull_list) {
					if (entry->input == sub_entry->input)
						break;
				}
				if (entry == NULL)
					JIVE_DEBUG_ASSERT(0);
			}
		}
	}

	/* check region hull whether it contains entries that don't belong there */
	jive_region_hull_entry * entry;
	JIVE_LIST_ITERATE(region->hull, entry, region_hull_list) {
		if (entry->input->origin()->node()->region->depth >= region->depth)
			JIVE_DEBUG_ASSERT(0);
	}
}

void
jive_region_verify_top_node_list(struct jive_region * region)
{
	jive_region * subregion;
	JIVE_LIST_ITERATE(region->subregions, subregion, region_subregions_list)
		jive_region_verify_top_node_list(subregion);

	/* check whether all nodes in the top_node_list are really nullary nodes */
	jive_node * node;
	JIVE_LIST_ITERATE(region->top_nodes, node, region_top_node_list)
		JIVE_DEBUG_ASSERT(node->ninputs == 0);

	/* check whether all nullary nodes from the region are in the top_node_list */
	JIVE_LIST_ITERATE(region->nodes, node, region_nodes_list) {
		if (node->ninputs != 0)
			continue;

		jive_node * top;
		JIVE_LIST_ITERATE(region->top_nodes, top, region_top_node_list) {
			if (top == node)
				break;
		}
		if (top == NULL)
			JIVE_DEBUG_ASSERT(0);
	}
}
#endif
