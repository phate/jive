/*
 * Copyright 2010 2011 2012 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_REGALLOC_CROSSING_ARC_H
#define JIVE_REGALLOC_CROSSING_ARC_H

#include <jive/common.h>

#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/shaped-node.h>
#include <jive/regalloc/shaped-region.h>
#include <jive/regalloc/shaped-variable.h>

#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/variable.h>

typedef struct jive_crossing_arc jive_crossing_arc;

struct jive_crossing_arc {
	jive_shaped_graph * shaped_graph;
	jive_shaped_node * origin_shaped_node;
	jive_shaped_node * target_shaped_node;
	jive_shaped_ssavar * shaped_ssavar;
	
	jive_shaped_node * start_shaped_node;
	jive_region * start_region;
	size_t boundary_region_depth;
};

JIVE_EXPORTED_INLINE void
jive_crossing_arc_init(jive_crossing_arc * self,
	jive_shaped_node * origin_shaped_node,
	jive_shaped_node * target_shaped_node,
	jive_shaped_ssavar * shaped_ssavar)
{
	self->shaped_graph = &shaped_ssavar->shaped_graph();
	self->origin_shaped_node = origin_shaped_node;
	self->target_shaped_node = target_shaped_node;
	self->shaped_ssavar = shaped_ssavar;
	
	if (!target_shaped_node
	|| (!self->origin_shaped_node
		&& shaped_ssavar->boundary_region_depth() > target_shaped_node->node()->region->depth()))
	{
		self->start_shaped_node = NULL;
		self->start_region = NULL;
	} else if (dynamic_cast<const jive::achr::type*>(&shaped_ssavar->ssavar().origin->type())) {
		jive_shaped_region * shaped_region = self->shaped_graph->map_region(
			shaped_ssavar->ssavar().origin->node()->region);
		self->start_shaped_node = shaped_region->last_in_region();
		self->start_region = shaped_ssavar->ssavar().origin->node()->region;
	} else {
		self->start_shaped_node = target_shaped_node->prev_in_region();
		self->start_region = target_shaped_node->node()->region;
	}
	
	if (origin_shaped_node) {
		self->boundary_region_depth = shaped_ssavar->ssavar().origin->node()->region->depth();
	} else {
		self->boundary_region_depth = shaped_ssavar->ssavar().origin->node()->region->depth();
		if (self->boundary_region_depth < shaped_ssavar->boundary_region_depth()) {
			self->boundary_region_depth = shaped_ssavar->boundary_region_depth();
		}
	}
}

typedef struct jive_crossing_arc_iterator jive_crossing_arc_iterator;

struct jive_crossing_arc_iterator {
	jive_shaped_node * node;
	jive_shaped_region * region;
	
	jive_shaped_graph * shaped_graph;
	jive_shaped_node * origin_shaped_node_;
	jive_region * current_region_;
	jive_region * exit_region_;
	size_t boundary_region_depth_;
};

JIVE_EXPORTED_INLINE void
jive_crossing_arc_iterator_init(
	jive_crossing_arc_iterator * self,
	jive_shaped_graph * shaped_graph,
	jive_shaped_node * origin_shaped_node,
	jive_shaped_node * target_shaped_node,
	jive_region * start_region,
	size_t boundary_region_depth)
{
	self->shaped_graph = shaped_graph;
	self->origin_shaped_node_ = origin_shaped_node;
	self->node = target_shaped_node;
	self->current_region_ = start_region;
	self->region =
		self->current_region_ ?
		shaped_graph->map_region(self->current_region_) : nullptr;
	self->exit_region_ = start_region;
	self->boundary_region_depth_ = boundary_region_depth;
}

JIVE_EXPORTED_INLINE void
jive_crossing_arc_iterator_init_ssavar(
	jive_crossing_arc_iterator * self,
	jive_shaped_node * origin_shaped_node,
	jive_shaped_node * target_shaped_node,
	jive_shaped_ssavar * shaped_ssavar)
{
	jive_crossing_arc arc;
	jive_crossing_arc_init(&arc, origin_shaped_node, target_shaped_node, shaped_ssavar);
	
	jive_crossing_arc_iterator_init(self, arc.shaped_graph, arc.origin_shaped_node,
		arc.start_shaped_node, arc.start_region, arc.boundary_region_depth);
	
	if (self->origin_shaped_node_ && self->node == self->origin_shaped_node_) {
		self->node = 0;
		self->current_region_ = 0;
		self->region = 0;
	}
	
}

JIVE_EXPORTED_INLINE void
jive_crossing_arc_iterator_next(jive_crossing_arc_iterator * self)
{
	if (self->node) {
		/* trace through next node or enter anchored sub-region */
		jive_node * node = self->node->node();
		self->node = self->node->prev_in_region();

		for (size_t n = 0; n < node->ninputs; n++) {
			jive::input * input = node->inputs[n];
			if (dynamic_cast<const jive::achr::type*>(&input->type())) {
				self->current_region_ = input->producer()->region;
				self->region = self->shaped_graph->map_region(self->current_region_);
				self->node = self->region->last_in_region();
				break;
			}
		}
	} else if (self->current_region_ == self->exit_region_) {
		/* trace out of region (unless desired depth reached)
		note: root region has depth 0, so implicitly terminates there */
		if (self->current_region_->depth() > self->boundary_region_depth_) {
			self->exit_region_ = self->exit_region_->parent;
			jive_shaped_node * anchor_shaped_node = self->shaped_graph->map_node(
				self->current_region_->anchor->node());
			
			self->node = anchor_shaped_node->prev_in_region();
			self->current_region_ = self->exit_region_;
			self->region = self->shaped_graph->map_region(self->current_region_);
		} else {
			self->node = 0;
			self->current_region_ = 0;
			self->region = 0;
		}
	} else {
		/* trace through neighbour regions or next node in parent region */
		size_t n = self->current_region_->anchor->index() + 1;
		jive_node * anchor_node = self->current_region_->anchor->node();
		jive::input * anchor = 0;
		while(n < anchor_node->ninputs) {
			jive::input * input = anchor_node->inputs[n];
			if (dynamic_cast<const jive::achr::type*>(&input->type())) {
				anchor = input;
				break;
			}
			n++;
		}
		if (anchor) {
			self->current_region_ = anchor->producer()->region;
			self->region = self->shaped_graph->map_region(self->current_region_);
			self->node = self->region->last_in_region();
		} else {
			jive_shaped_node * anchor_shaped_node = self->shaped_graph->map_node(
				anchor_node);
			self->node = anchor_shaped_node->prev_in_region();
			self->current_region_ = self->current_region_->parent;
			self->region = self->shaped_graph->map_region(self->current_region_);
		}
	}
	
	if (self->origin_shaped_node_ && self->node == self->origin_shaped_node_) {
		self->node = 0;
		self->current_region_ = 0;
		self->region = 0;
	}
}

#endif
