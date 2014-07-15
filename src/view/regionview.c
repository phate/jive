/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/region.h>
#include <jive/vsdg/graph.h>
#include <jive/view/nodeview.h>
#include <jive/view/regionview.h>
#include <jive/view/graphview.h>
#include <jive/view/reservationtracker.h>
#include <jive/util/list.h>

void
jive_regionview_init_(jive_regionview * self, jive_graphview * graphview, jive_region * region)
{
	self->region = region;
	self->x = 0;
	self->start_row_index = 0;
	self->end_row_index = 0;
	self->width = 0;
	self->subregions.first = self->subregions.last = 0;
	self->nodes.first = self->nodes.last = 0;
	self->regionview_subregions_list.prev = self->regionview_subregions_list.next = 0;
	self->graphview = graphview;
}

void
jive_regionview_fini_(jive_regionview * self)
{
	jive_nodeview * nodeview = self->nodes.first;
	while(nodeview) {
		jive_nodeview * tmp = nodeview->regionview_nodes_list.next;
		jive_nodeview_destroy(nodeview);
		nodeview = tmp;
	}
	
	jive_regionview * sub = self->subregions.first;
	while(sub) {
		jive_regionview * tmp = sub->regionview_subregions_list.next;
		jive_regionview_destroy(sub);
		sub = tmp;
	}
}

jive_regionview *
jive_regionview_create(jive_graphview * graphview, jive_region * region)
{
	jive_regionview * regionview = new jive_regionview;
	jive_regionview_init_(regionview, graphview, region);
	return regionview;
}

void
jive_regionview_destroy(jive_regionview * self)
{
	jive_regionview_fini_(self);
	delete self;
}

void
jive_regionview_move_horizontal(jive_regionview * self, int offset)
{
	self->x = self->x + offset;
	jive_nodeview * nodeview = self->nodes.first;
	while(nodeview) {
		nodeview->x += offset;
		nodeview = nodeview->regionview_nodes_list.next;
	}
	
	jive_regionview * sub = self->subregions.first;
	while(sub) {
		jive_regionview_move_horizontal(sub, offset);
		sub = sub->regionview_subregions_list.next;
	}
}

static void
jive_regionview_layout_nodes_recursive(jive_regionview * self, jive_nodeview * nodeview, jive_reservationtracker * reservation)
{
	if (nodeview->placed) return;
	
	jive_nodeview_layout(nodeview, reservation);
	size_t n;
	
	for(n=0; n<nodeview->node->ninputs; n++) {
		jive::input * input = nodeview->node->inputs[n];
		if (input->producer()->region != self->region) continue;
		jive_nodeview * nodeview = jive_nodeview_map_lookup(&self->graphview->nodemap,
			input->producer());
		jive_regionview_layout_nodes_recursive(self, nodeview, reservation);
	}
	for(n=0; n<nodeview->node->noutputs; n++) {
		jive::input * user = nodeview->node->outputs[n]->users.first;
		while(user) {
			if (user->node->region == self->region) {
				jive_nodeview * nodeview = jive_nodeview_map_lookup(&self->graphview->nodemap, user->node);
				jive_regionview_layout_nodes_recursive(self, nodeview, reservation);
			}
			user = user->output_users_list.next;
		}
	}
}

void
jive_regionview_layout(jive_regionview * self, jive_reservationtracker * parent_reservation)
{
	jive_reservationtracker reservation;
	jive_reservationtracker_init(&reservation);
	
	jive_region * subregion;
	JIVE_LIST_ITERATE(self->region->subregions, subregion, region_subregions_list) {
		jive_regionview * subregionview = jive_regionview_create(self->graphview, subregion);
		jive_regionview_layout(subregionview, &reservation);
		JIVE_LIST_PUSH_BACK(self->subregions, subregionview, regionview_subregions_list);
	}
	
	int min_y = reservation.min_y;
	int max_y = reservation.max_y;
	
	jive_node * node;
	JIVE_LIST_ITERATE(self->region->nodes, node, region_nodes_list) {
		jive_nodeview * nodeview = jive_nodeview_map_lookup(&self->graphview->nodemap, node);
		JIVE_LIST_PUSH_BACK(self->nodes, nodeview, regionview_nodes_list);
		jive_regionview_layout_nodes_recursive(self, nodeview, &reservation);
		if (node->depth_from_root + 1 > max_y) max_y = node->depth_from_root + 1;
		if (node->depth_from_root < min_y) min_y = node->depth_from_root;
	}
	
	if (min_y > max_y) {
		/* this means that the region is completely empty */
		min_y = 0;
		max_y = 1;
	}
	
	int width = reservation.max_x - reservation.min_x;
	int height = max_y - min_y;
	int region_pad = 1;
	
	jive_reservationrect rect = {0, min_y, width + region_pad * 2, height};
	int x = jive_reservationtracker_find_space(parent_reservation, 1, &rect, 0);
	jive_reservationtracker_reserve_boxes(parent_reservation, 1, &rect, x);
	
	self->start_row_index = min_y;
	self->end_row_index = max_y;
	self->x = reservation.min_x - 1;
	self->width = reservation.max_x - reservation.min_x + 1;
	jive_regionview_move_horizontal(self, x + region_pad - reservation.min_x);
	
	jive_graphview_row * min_row = jive_graphview_get_row(self->graphview, min_y);
	min_row->pad_above += 1;
	self->upper_border_offset = - min_row->pad_above;
	
	jive_graphview_row * max_row = jive_graphview_get_row(self->graphview, max_y - 1);
	self->lower_border_offset = max_row->pad_below;
	max_row->pad_below ++;
	
	jive_reservationtracker_fini(&reservation);
}

void
jive_regionview_draw(jive_regionview * self, jive_textcanvas * dst)
{
	jive_regionview * subregion;
	JIVE_LIST_ITERATE(self->subregions, subregion, regionview_subregions_list)
		jive_regionview_draw(subregion, dst);
	
	jive_graphview_row * start_row, * end_row;
	start_row = jive_graphview_get_row(self->graphview, self->start_row_index);
	end_row = jive_graphview_get_row(self->graphview, self->end_row_index - 1);
	int y_start = start_row->y + self->upper_border_offset;
	int y_end = end_row->y + end_row->height + self->lower_border_offset;
	
	jive_textcanvas_box(dst, self->x, y_start, self->x + self->width, y_end, false, true);
	
	jive_nodeview * nodeview;
	JIVE_LIST_ITERATE(self->nodes, nodeview, regionview_nodes_list)
		jive_nodeview_draw(nodeview, dst);
}
