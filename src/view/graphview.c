/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/view/regionview.h>
#include <jive/view/graphview.h>
#include <jive/view/reservationtracker.h>
#include <jive/util/list.h>

/* graphview_row */

jive_graphview_row::jive_graphview_row() noexcept
	: width(0)
	, height(0)
	, pad_above(0)
	, pad_below(0)
	, x(0)
	, y(0)
{}

/* graphview */

jive_graphview::~jive_graphview() noexcept
{
	jive_textcanvas_fini(&canvas);
	delete root_region;
}

static void
jive_graphview_add_node_recursive(jive_graphview * self, jive_node * node)
{
	if (self->nodemap.find(node) != self->nodemap.end())
		return;
	
	jive_nodeview * nodeview = new jive_nodeview(self, node);
	self->nodemap.insert(std::make_pair(node, nodeview));
	for (size_t n = 0; n < node->noutputs; n++) {
		jive_outputview * outputview = nodeview->outputs[n];
		self->outputmap.insert(std::make_pair(node->outputs[n], outputview));
	}
	
	for (size_t n = 0; n < node->ninputs; n++) {
		jive::input * input = node->inputs[n];
		jive_inputview * inputview = nodeview->inputs[n];
		self->inputmap.insert(std::make_pair(input, inputview));
		jive_graphview_add_node_recursive(self, input->producer());
	}
}

jive_graphview::jive_graphview(jive_graph * graph_)
	: graph(graph_)
	, width(0)
	, height(0)
	, root_region(nullptr)
{
	jive_node * node;
	JIVE_LIST_ITERATE(graph->bottom, node, graph_bottom_list)
		jive_graphview_add_node_recursive(this, node);
}

jive_graphview_row *
jive_graphview_get_row(jive_graphview * self, size_t index)
{
	if (index >= self->rows.size())
		self->rows.resize(index+1);

	return &self->rows[index];
}

void
jive_graphview_layout(jive_graphview * self)
{
	jive_reservationtracker reservation;
	
	/* compute sizes of regions and nodes */
	jive_regionview * regionview = new jive_regionview(self, self->graph->root_region);
	jive_regionview_layout(regionview, &reservation);
	self->root_region = regionview;
	
	self->width = reservation.max_x - reservation.min_x;
	jive_regionview_move_horizontal(regionview, reservation.min_x);
	
	/* compute sizes of rows and vertical positions, as
	well as total size of graph */
	for (size_t n = 0; n < self->rows.size(); n++) {
		jive_graphview_row * row = &self->rows[n];
		row->x = 0;
		row->y = self->height + row->pad_above;
		self->height += row->height + row->pad_above + row->pad_below;
	}
}

void
jive_graphview_draw(jive_graphview * self)
{
	jive_graphview_layout(self);
	jive_textcanvas_init(&self->canvas, self->width, self->height);
	
	jive_regionview_draw(self->root_region, &self->canvas);

	for (auto i : self->inputmap) {
		jive_inputview * inputview = i.second;
		jive_outputview * outputview = self->outputmap[inputview->input->origin()];
		
		JIVE_DEBUG_ASSERT(outputview->nodeview->placed);
		int begin_x = outputview->nodeview->x + jive_outputview_get_edge_offset(outputview);
		int begin_y = jive_nodeview_get_y(outputview->nodeview) + outputview->nodeview->height - 1;
		int bend_y = jive_nodeview_get_y(inputview->nodeview) + inputview->edge_bend_y;
		int end_x = inputview->nodeview->x + jive_inputview_get_edge_offset(inputview);
		int end_y = jive_nodeview_get_y(inputview->nodeview);
		
		jive_textcanvas_vline(&self->canvas, begin_x, begin_y, bend_y, false, false);
		jive_textcanvas_hline(&self->canvas, begin_x, bend_y, end_x, false, false);
		jive_textcanvas_vline(&self->canvas, end_x, bend_y, end_y, false, false);
	}
}
