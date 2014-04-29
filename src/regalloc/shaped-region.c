/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/regalloc/shaped-region.h>

#include <jive/context.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/shaped-node-private.h>
#include <jive/regalloc/shaped-region-private.h>
#include <jive/regalloc/xpoint-private.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/region-ssavar-use-private.h>

static jive_cut *
jive_cut_create(jive_context * context, jive_shaped_region * shaped_region, jive_cut * before)
{
	jive_cut * self = jive_context_malloc(context, sizeof(*self));
	
	self->shaped_region = shaped_region;
	JIVE_LIST_INSERT(shaped_region->cuts, before, self, region_cut_list);
	self->locations.first = self->locations.last = 0;
	
	return self;
}

JIVE_DEFINE_HASH_TYPE(jive_shaped_region_hash, jive_shaped_region, struct jive_region *, region, hash_chain);

jive_shaped_region *
jive_shaped_region_create(struct jive_shaped_graph * shaped_graph, struct jive_region * region)
{
	jive_context * context = shaped_graph->context;
	jive_shaped_region * self = jive_context_malloc(context, sizeof(*self));
	
	self->shaped_graph = shaped_graph;
	self->region = region;
	self->cuts.first = self->cuts.last = NULL;
	jive_region_varcut_init(&self->active_top, self);
	
	jive_shaped_region_hash_insert(&shaped_graph->region_map, self);
	
	return self;
}

jive_cut *
jive_shaped_region_create_cut(jive_shaped_region * self)
{
	return jive_cut_create(self->shaped_graph->context, self, self->cuts.first);
}

jive_shaped_node *
jive_shaped_region_first(const jive_shaped_region * self)
{
	jive_cut * cut = self->cuts.first;
	while(cut && !cut->locations.first)
		cut = cut->region_cut_list.next;
	if (cut) return cut->locations.first;
	return 0;
}

jive_shaped_node *
jive_shaped_region_last(const jive_shaped_region * self)
{
	jive_cut * cut = self->cuts.last;
	while(cut && !cut->locations.last)
		cut = cut->region_cut_list.prev;
	if (cut) return cut->locations.last;
	return 0;
}

void
jive_shaped_region_destroy_cuts(jive_shaped_region * self)
{
	while(self->cuts.first)
		jive_cut_destroy(self->cuts.first);
}

void
jive_shaped_region_destroy(jive_shaped_region * self)
{
	jive_shaped_region_destroy_cuts(self);
	jive_region_varcut_fini(&self->active_top);
	jive_shaped_region_hash_remove(&self->shaped_graph->region_map, self);
	jive_context_free(self->shaped_graph->context, self);
}

void
jive_cut_destroy(jive_cut * self)
{
	jive_context * context = self->shaped_region->shaped_graph->context;
	
	while(self->locations.first)
		jive_shaped_node_destroy(self->locations.first);
	
	JIVE_LIST_REMOVE(self->shaped_region->cuts, self, region_cut_list);
	
	jive_context_free(context, self);
}

jive_cut *
jive_cut_create_above(jive_cut * self)
{
	return jive_cut_create(self->shaped_region->shaped_graph->context, self->shaped_region, self);
}

jive_cut *
jive_cut_create_below(jive_cut * self)
{
	return jive_cut_create(self->shaped_region->shaped_graph->context, self->shaped_region, self->region_cut_list.next);
}

jive_cut *
jive_cut_split(jive_cut * self, jive_shaped_node * at)
{
	JIVE_DEBUG_ASSERT(at == NULL || at->cut == self);
	
	if (at != self->locations.first) {
		jive_cut * above = jive_cut_create_above(self);
		while (at != self->locations.first) {
			jive_shaped_node * loc = self->locations.first;
			
			JIVE_LIST_REMOVE(self->locations, loc, cut_location_list);
			JIVE_LIST_PUSH_BACK(above->locations, loc, cut_location_list);
			loc->cut = above;
		}
	}
	if (self->locations.last) {
		jive_cut * below = jive_cut_create_below(self);
		while (self->locations.last) {
			jive_shaped_node * loc = self->locations.last;
			
			JIVE_LIST_REMOVE(self->locations, loc, cut_location_list);
			JIVE_LIST_PUSH_FRONT(below->locations, loc, cut_location_list);
			loc->cut = below;
		}
	}
	
	return self;
}

static void
add_crossings_from_lower_location(jive_shaped_graph * shaped_graph, jive_shaped_node * shaped_node, jive_shaped_node * lower)
{
	struct jive_nodevar_xpoint_hash_byssavar_iterator i;
	JIVE_HASH_ITERATE(jive_nodevar_xpoint_hash_byssavar, lower->ssavar_xpoints, i) {
		jive_nodevar_xpoint * xpoint = i.entry;
		if (!xpoint->before_count) continue;
		jive_ssavar * ssavar = xpoint->shaped_ssavar->ssavar;
		if (ssavar->origin->node == shaped_node->node) {
			jive_shaped_node_add_ssavar_after(shaped_node, xpoint->shaped_ssavar, ssavar->variable, xpoint->before_count);
		} else {
			if (jive_output_isinstance(ssavar->origin, &JIVE_ANCHOR_OUTPUT)) continue;
			if (xpoint->shaped_ssavar->boundary_region_depth > shaped_node->node->region->depth && ! jive_shaped_graph_map_node(shaped_graph, ssavar->origin->node)) continue;
			jive_shaped_node_add_ssavar_crossed(shaped_node, xpoint->shaped_ssavar, ssavar->variable, xpoint->before_count);
		}
	}
	
	size_t n;
	for(n = 0; n < lower->node->ninputs; n++) {
		jive_input * input = lower->node->inputs[n];
		if (!dynamic_cast<jive_anchor_input*>(input)) continue;
		jive_shaped_region * shaped_region = jive_shaped_graph_map_region(shaped_graph, input->origin->node->region);
		
		/* if this is a control edge, pass through variables from the top
		of the subregion */
		add_crossings_from_lower_location(shaped_graph, shaped_node, jive_shaped_region_first(shaped_region));
		/* variables passed through both the anchor node and the
		region would be counted twice, so remove the duplicates */
		JIVE_HASH_ITERATE(jive_nodevar_xpoint_hash_byssavar, lower->ssavar_xpoints, i) {
			jive_nodevar_xpoint * xpoint = i.entry;
			if (!xpoint->cross_count) continue;
			jive_shaped_node_remove_ssavar_crossed(shaped_node, xpoint->shaped_ssavar,
				xpoint->shaped_ssavar->ssavar->variable, xpoint->cross_count);
		}
	}
}

struct jive_shaped_node *
jive_cut_insert(jive_cut * self, jive_shaped_node * before, jive_node * node)
{
	jive_shaped_graph * shaped_graph = self->shaped_region->shaped_graph;
	
	size_t n;
	
	/* set aside crossings for ssavars originating here */
	for(n = 0; n < node->noutputs; n++) {
		jive_output * output = node->outputs[n];
		jive_ssavar * ssavar;
		
		JIVE_LIST_ITERATE(output->originating_ssavars, ssavar, originating_ssavar_list) {
			jive_shaped_ssavar * shaped_ssavar = jive_shaped_graph_map_ssavar(shaped_graph, ssavar);
			
			jive_input * input;
			JIVE_LIST_ITERATE(ssavar->assigned_inputs, input, ssavar_input_list) {
				jive_shaped_ssavar_xpoints_unregister_arc(shaped_ssavar, input, output);
			}
			struct jive_ssavar_region_hash_iterator i;
			JIVE_HASH_ITERATE(jive_ssavar_region_hash, ssavar->assigned_regions, i) {
				jive_region * region = i.entry->region;
				jive_shaped_ssavar_xpoints_unregister_region_arc(shaped_ssavar, output, region);
			}
			
			shaped_ssavar->boundary_region_depth = (size_t) -1;
		}
	}
	
	jive_shaped_node * shaped_node = jive_shaped_node_create(self, node);
	
	JIVE_LIST_INSERT(self->locations, before, shaped_node, cut_location_list);
	
	jive_shaped_node * next = jive_shaped_node_next_in_region(shaped_node);
	if (next) {
		add_crossings_from_lower_location(shaped_graph, shaped_node, next);
	} else if (node->region->anchor) {
		next = jive_shaped_graph_map_node(shaped_graph, node->region->anchor->node);
		struct jive_nodevar_xpoint_hash_byssavar_iterator i;
		JIVE_HASH_ITERATE(jive_nodevar_xpoint_hash_byssavar, next->ssavar_xpoints, i) {
			jive_nodevar_xpoint * xpoint = i.entry;
			jive_shaped_node_add_ssavar_crossed(shaped_node, xpoint->shaped_ssavar,
				xpoint->shaped_ssavar->ssavar->variable, xpoint->cross_count);
		}
	}
	
	for(n = 0; n < node->ninputs; n++) {
		jive_input * input = node->inputs[n];
		if (!dynamic_cast<jive_anchor_input*>(input))
			continue;
		struct jive_nodevar_xpoint_hash_byssavar_iterator i;
		JIVE_HASH_ITERATE(jive_nodevar_xpoint_hash_byssavar, shaped_node->ssavar_xpoints, i) {
			jive_nodevar_xpoint * xpoint = i.entry;
			jive_shaped_region * shaped_region = jive_shaped_graph_map_region(shaped_graph, input->origin->node->region);
			jive_shaped_region_add_active_top(shaped_region, xpoint->shaped_ssavar, xpoint->cross_count);
		}
	}
	
	/* reinstate crossings for ssavars originating here */
	for(n = 0; n < node->noutputs; n++) {
		jive_output * output = node->outputs[n];
		jive_ssavar * ssavar;
		
		JIVE_LIST_ITERATE(output->originating_ssavars, ssavar, originating_ssavar_list) {
			jive_shaped_ssavar * shaped_ssavar = jive_shaped_graph_map_ssavar(shaped_graph, ssavar);
			
			jive_input * input;
			JIVE_LIST_ITERATE(ssavar->assigned_inputs, input, ssavar_input_list) {
				jive_shaped_ssavar_xpoints_register_arc(shaped_ssavar, input, output);
			}
			struct jive_ssavar_region_hash_iterator i;
			JIVE_HASH_ITERATE(jive_ssavar_region_hash, ssavar->assigned_regions, i) {
				jive_region * region = i.entry->region;
				jive_shaped_ssavar_xpoints_register_region_arc(shaped_ssavar, output, region);
			}
			
			if (ssavar->assigned_output)
				jive_shaped_node_add_ssavar_after(shaped_node, shaped_ssavar, ssavar->variable, 1);
		}
	}
	
	/* add crossings for ssavars used here */
	for(n = 0; n < node->ninputs; n++) {
		jive_input * input = node->inputs[n];
		if (!input->ssavar) continue;
		jive_shaped_ssavar * shaped_ssavar = jive_shaped_graph_map_ssavar(shaped_graph, input->ssavar);
		jive_shaped_ssavar_xpoints_register_arc(shaped_ssavar, input, input->origin);
	}
	
	/* if this is the bottom node of a loop region, need to register
	crossings on behalf of this region */
	if (node == jive_region_get_bottom_node(node->region)) {
		struct jive_region_ssavar_hash_iterator i;
		JIVE_HASH_ITERATE(jive_region_ssavar_hash, node->region->used_ssavars, i) {
			jive_ssavar * ssavar = i.entry->ssavar;
			jive_shaped_ssavar * shaped_ssavar = jive_shaped_graph_map_ssavar(shaped_graph, ssavar);
			jive_shaped_ssavar_xpoints_register_region_arc(shaped_ssavar, ssavar->origin, node->region);
		}
	}
	
	jive_node_notifier_slot_call(&shaped_graph->on_shaped_node_create, node);
	return shaped_node;
}
