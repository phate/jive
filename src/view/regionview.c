/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/util/list.h>
#include <jive/view/graphview.h>
#include <jive/view/nodeview.h>
#include <jive/view/regionview.h>
#include <jive/view/reservationtracker.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/region.h>

jive_regionview::~jive_regionview() noexcept
{
	for (auto nodeview : nodes)
		delete nodeview;

	for (auto regionview : subregions)
		delete regionview;
}

jive_regionview::jive_regionview(jive_graphview * graphview_, const jive::region * region_)
	: region(region_)
	, graphview(graphview_)
	, x(0)
	, width(0)
	, start_row_index(0)
	, end_row_index(0)
	, upper_border_offset(0)
	, lower_border_offset(0)
{}

void
jive_regionview_move_horizontal(jive_regionview * self, int offset)
{
	self->x = self->x + offset;
	for (auto nodeview : self->nodes)
		nodeview->x += offset;

	for (auto regionview : self->subregions)
		jive_regionview_move_horizontal(regionview, offset);
}

static void
jive_regionview_layout_nodes_recursive(jive_regionview * self, jive_nodeview * nodeview,
	jive_reservationtracker * reservation)
{
	if (nodeview->placed) return;
	
	jive_nodeview_layout(nodeview, reservation);

	for(size_t n = 0; n < nodeview->node->ninputs(); n++) {
		jive::iport * input = nodeview->node->input(n);
		if (input->origin()->region() != self->region)
			continue;
		if (!input->origin()->node())
			continue;

		jive_nodeview * nodeview = self->graphview->nodemap[input->origin()->node()];
		jive_regionview_layout_nodes_recursive(self, nodeview, reservation);
	}
	for(size_t n = 0; n < nodeview->node->noutputs(); n++) {
		for (auto user : nodeview->node->output(n)->users) {
			if (user->node() && user->node()->region() == self->region) {
				jive_nodeview * nodeview = self->graphview->nodemap[user->node()];
				jive_regionview_layout_nodes_recursive(self, nodeview, reservation);
			}
		}
	}
}

void
jive_regionview_layout(jive_regionview * self, jive_reservationtracker * parent_reservation)
{
	jive_reservationtracker reservation;
	
	jive::region * subregion;
	JIVE_LIST_ITERATE(self->region->subregions, subregion, region_subregions_list) {
		jive_regionview * subregionview = new jive_regionview(self->graphview, subregion);
		jive_regionview_layout(subregionview, &reservation);
		self->subregions.push_back(subregionview);
	}
	
	int min_y = reservation.min_y;
	int max_y = reservation.max_y;

	for (auto & node : self->region->nodes)	{
		jive_nodeview * nodeview = self->graphview->nodemap[&node];
		self->nodes.push_back(nodeview);
		jive_regionview_layout_nodes_recursive(self, nodeview, &reservation);
		if ((ssize_t)node.depth()+1 > max_y) max_y = node.depth()+1;
		if ((ssize_t)node.depth() < min_y) min_y = node.depth();
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
}

void
jive_regionview_draw(jive_regionview * self, jive_textcanvas * dst)
{
	for (auto regionview : self->subregions)
		jive_regionview_draw(regionview, dst);
	
	jive_graphview_row * start_row, * end_row;
	start_row = jive_graphview_get_row(self->graphview, self->start_row_index);
	end_row = jive_graphview_get_row(self->graphview, self->end_row_index - 1);
	int y_start = start_row->y + self->upper_border_offset;
	int y_end = end_row->y + end_row->height + self->lower_border_offset;
	
	jive_textcanvas_box(dst, self->x, y_start, self->x + self->width, y_end, false, true);
	
	for (auto nodeview : self->nodes)
		jive_nodeview_draw(nodeview, dst);
}
