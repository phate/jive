/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/regalloc/shaped-node-private.h>
#include <jive/regalloc/shaped-node.h>

#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/xpoint-private.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/resource-private.h>

jive_shaped_node *
jive_shaped_node_create(jive_cut * cut, jive_node * node)
{
	jive_shaped_graph * shaped_graph = cut->shaped_region->shaped_graph;
	JIVE_DEBUG_ASSERT(
		!node->region->anchor ||
		jive_shaped_graph_map_node(shaped_graph, node->region->anchor->node()));
	
	jive_shaped_node * self = new jive_shaped_node;
	
	self->shaped_graph = shaped_graph;
	self->node = node;
	
	shaped_graph->node_map.insert(self);

	self->cut = cut;
	
	jive_resource_class_count_init(&self->use_count_before);
	jive_resource_class_count_init(&self->use_count_after);
	
	return self;
}

jive_shaped_node *
jive_shaped_node_prev_in_region(const jive_shaped_node * self)
{
	jive_shaped_node * tmp = self->cut_location_list.prev;
	jive_cut * cut = self->cut->region_cut_list.prev;
	
	while(cut && !tmp) {
		tmp = cut->locations.last;
		cut = cut->region_cut_list.prev;
	}
	
	return tmp;
}

jive_shaped_node *
jive_shaped_node_next_in_region(const jive_shaped_node * self)
{
	jive_shaped_node * tmp = self->cut_location_list.next;
	jive_cut * cut = self->cut->region_cut_list.next;
	
	while(cut && !tmp) {
		tmp = cut->locations.first;
		cut = cut->region_cut_list.next;
	}
	
	return tmp;
}

void
jive_shaped_node_get_active_before(const jive_shaped_node * self, jive_mutable_varcut * cut)
{
	jive_mutable_varcut_clear(cut);
	for (const jive_nodevar_xpoint & xpoint : self->ssavar_xpoints) {
		if (xpoint.before_count) {
			jive_mutable_varcut_ssavar_add(cut, xpoint.shaped_ssavar, xpoint.before_count);
		}
	}
}

void
jive_shaped_node_get_active_after(const jive_shaped_node * self, jive_mutable_varcut * cut)
{
	jive_mutable_varcut_clear(cut);
	for (const jive_nodevar_xpoint & xpoint : self->ssavar_xpoints) {
		if (xpoint.after_count) {
			jive_mutable_varcut_ssavar_add(cut, xpoint.shaped_ssavar, xpoint.after_count);
		}
	}
}

static void
jive_shaped_node_remove_all_crossed(jive_shaped_node * self)
{
	auto i = self->ssavar_xpoints.begin();
	while (i != self->ssavar_xpoints.end()) {
		jive_nodevar_xpoint * xpoint = i.ptr();
		++i;
		jive_shaped_node_remove_ssavar_crossed(
			self,
			xpoint->shaped_ssavar,
			xpoint->shaped_ssavar->ssavar->variable,
			xpoint->cross_count);
	}
}

void
jive_shaped_node_destroy(jive_shaped_node * self)
{
	self->shaped_graph->on_shaped_node_destroy(self->node);
	
	/* destroy node shapes of all anchored regions */
	size_t n;
	for(n = 0; n < self->node->ninputs; n++)
	{
		jive::input * input = self->node->inputs[n];
		if (dynamic_cast<const jive::achr::type*>(&input->type())) {
			jive_region * region = input->producer()->region;
			jive_shaped_region * shaped_region = jive_shaped_graph_map_region(self->shaped_graph, region);
			jive_shaped_region_destroy_cuts(shaped_region);
		}
	}
	
	/* remove things that cross this node */
	jive_shaped_node_remove_all_crossed(self);
	
	/* set aside crossings of vars beginning or ending here */
	for(n = 0; n < self->node->ninputs; n++) {
		jive::input * input = self->node->inputs[n];
		jive_ssavar * ssavar = input->ssavar;
		if (!ssavar) continue;
		jive_shaped_ssavar_xpoints_unregister_arc(
			jive_shaped_graph_map_ssavar(self->shaped_graph, ssavar), input, input->origin());
	}
	for(n = 0; n < self->node->noutputs; n++) {
		jive::output * output = self->node->outputs[n];
		jive::input * user;
		JIVE_LIST_ITERATE(output->users, user, output_users_list) {
			jive_ssavar * ssavar = user->ssavar;
			if (!ssavar) continue;
			jive_shaped_ssavar_xpoints_unregister_arc(
				jive_shaped_graph_map_ssavar(self->shaped_graph, ssavar),
				user,
				output);
		}
		jive_ssavar * ssavar = output->ssavar;
		if (!ssavar) continue;
		jive_shaped_node_remove_ssavar_after(
			self,
			jive_shaped_graph_map_ssavar(self->shaped_graph, ssavar),
			ssavar->variable,
			1);
	}
	
	/* if this is the bottom node of a loop region, unregister
	crossings on behalf of this region */
	if (self->node == jive_region_get_bottom_node(self->node->region)) {
		for (jive_region_ssavar_use & use : self->node->region->used_ssavars) {
			jive_ssavar * ssavar = use.ssavar;
			jive_shaped_ssavar * shaped_ssavar = jive_shaped_graph_map_ssavar(self->shaped_graph, ssavar);
			jive_shaped_ssavar_xpoints_unregister_region_arc(
				shaped_ssavar,
				ssavar->origin,
				self->node->region);
		}
	}
	
	/* remove from graph */
	self->shaped_graph->node_map.erase(self);
	JIVE_LIST_REMOVE(self->cut->locations, self, cut_location_list);
	
	/* reinstate crossings for those arcs that have this node as origin */
	for(n = 0; n < self->node->noutputs; n++) {
		jive::output * output = self->node->outputs[n];
		jive::input * user;
		JIVE_LIST_ITERATE(output->users, user, output_users_list) {
			jive_ssavar * ssavar = user->ssavar;
			if (!ssavar) continue;
			jive_shaped_ssavar_xpoints_register_arc(
				jive_shaped_graph_map_ssavar(self->shaped_graph, ssavar),
				user, output);
		}
	}
	
	JIVE_DEBUG_ASSERT(self->ssavar_xpoints.size() == 0);
	jive_resource_class_count_fini(&self->use_count_before);
	jive_resource_class_count_fini(&self->use_count_after);
	
	delete self;
}

void
jive_shaped_node_downward_iterator_init(
	jive_shaped_node_downward_iterator * self,
	jive_shaped_node * start)
{
	self->shaped_graph = start->shaped_graph;
	self->current = start;
	self->leave_region_depth = start->cut->shaped_region->region->depth;
	self->boundary_region_depth = self->leave_region_depth;
}

void
jive_shaped_node_downward_iterator_init_outward(
	jive_shaped_node_downward_iterator * self,
	jive_shaped_node * start)
{
	self->shaped_graph = start->shaped_graph;
	self->current = start;
	self->leave_region_depth = start->cut->shaped_region->region->depth;
	self->boundary_region_depth = 0;
}

jive_shaped_node *
jive_shaped_node_downward_iterator_next(jive_shaped_node_downward_iterator * self)
{
	if (!self->current)
		return NULL;
	
	jive_shaped_node * current = self->current;
	jive_shaped_region * shaped_region = current->cut->shaped_region;
	jive_shaped_node * next = jive_shaped_node_next_in_region(current);
	bool enter_subregions = true;
	if (!next) {
		if (shaped_region->region->depth > self->leave_region_depth) {
			jive_node * anchor_node = shaped_region->region->anchor->node();
			size_t n = shaped_region->region->anchor->index() + 1;
			while (n < anchor_node->ninputs) {
				jive::input * input = anchor_node->inputs[n];
				if (dynamic_cast<const jive::achr::type*>(&input->type())) {
					jive_shaped_region * sub = jive_shaped_graph_map_region(self->shaped_graph,
						input->producer()->region);
					next = jive_shaped_region_first(sub);
					if (next)
						break;
				}
				n = n + 1;
			}
			if (!next && shaped_region->region->depth > self->boundary_region_depth) {
				enter_subregions = false;
				next = jive_shaped_graph_map_node(self->shaped_graph, shaped_region->region->anchor->node());
				self->leave_region_depth = shaped_region->region->anchor->node()->region->depth;
			}
		} else if (shaped_region->region->depth > self->boundary_region_depth) {
			enter_subregions = false;
			next = jive_shaped_graph_map_node(self->shaped_graph, shaped_region->region->anchor->node());
			self->leave_region_depth = shaped_region->region->anchor->node()->region->depth;
		}
	}
	if (next && enter_subregions) {
		size_t n = 0;
		while (n < next->node->ninputs) {
			jive::input * input = next->node->inputs[n];
			if (dynamic_cast<const jive::achr::type*>(&input->type())) {
				jive_shaped_region * sub = jive_shaped_graph_map_region(self->shaped_graph,
					input->producer()->region);
				jive_shaped_node * tmp = jive_shaped_region_first(sub);
				if (tmp) {
					next = tmp;
					n = 0;
					continue;
				}
			}
			n = n + 1;
		}
	}
	
	self->current = next;
	
	return current;
}

void
jive_shaped_node_downward_iterator_fini(jive_shaped_node_downward_iterator * self)
{
	(void)self;
}

