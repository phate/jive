#include <jive/view/regionview.h>
#include <jive/view/graphview.h>
#include <jive/view/reservationtracker.h>
#include <jive/util/list.h>

static void
jive_graphview_row_init(jive_graphview_row * self)
{
	self->width = 0;
	self->height = 0;
	self->pad_above = 0;
	self->pad_below = 0;
	self->x = 0;
	self->y = 0;
}

static void
jive_graphview_add_node_recursive(jive_graphview * self, jive_node * node)
{
	if (jive_nodeview_map_lookup(&self->nodemap, node)) return;
	
	jive_nodeview * nodeview = jive_nodeview_create(self, node);
	jive_nodeview_map_set(&self->nodemap, node, nodeview);
	size_t n;
	for(n=0; n<node->noutputs; n++) {
		jive_output * output = node->outputs[n];
		jive_outputview * outputview = nodeview->outputs[n];
		jive_outputview_map_set(&self->outputmap, output, outputview);
	}
	
	for(n=0; n<node->ninputs; n++) {
		jive_input * input = node->inputs[n];
		jive_inputview * inputview = nodeview->inputs[n];
		jive_inputview_map_set(&self->inputmap, input, inputview);
		jive_graphview_add_node_recursive(self, input->origin->node);
	}
}

static void
jive_graphview_init(jive_graphview * self, jive_graph * graph)
{
	self->graph = graph;
	jive_nodeview_map_init(&self->nodemap, graph->context);
	jive_inputview_map_init(&self->inputmap, graph->context);
	jive_outputview_map_init(&self->outputmap, graph->context);
	self->width = 0;
	self->height = 0;
	self->nrows = 0;
	self->rows = 0;
	self->root_region = 0;
	
	jive_node * node;
	JIVE_LIST_ITERATE(graph->bottom, node, graph_bottom_list)
		jive_graphview_add_node_recursive(self, node);
}

static void
jive_graphview_fini(jive_graphview * self)
{
	jive_textcanvas_fini(&self->canvas);
	jive_context_free(self->graph->context, self->rows);
	jive_regionview_destroy(self->root_region);
	jive_nodeview_map_fini(&self->nodemap);
	jive_inputview_map_fini(&self->inputmap);
	jive_outputview_map_fini(&self->outputmap);
}

jive_graphview *
jive_graphview_create(jive_graph * graph)
{
	jive_graphview * graphview = jive_context_malloc(graph->context, sizeof(*graphview));
	jive_graphview_init(graphview, graph);
	return graphview;
}

void
jive_graphview_destroy(jive_graphview * self)
{
	jive_graphview_fini(self);
	jive_context_free(self->graph->context, self);
}

jive_graphview_row *
jive_graphview_get_row(jive_graphview * self, size_t index)
{
	if (index >= self->nrows) {
		self->rows = jive_context_realloc(self->graph->context, self->rows,
			sizeof(*self->rows) * (index + 1));
		size_t n;
		for(n = self->nrows; n<=index; n++) jive_graphview_row_init(&self->rows[n]);
		self->nrows = index + 1;
	}
	return &self->rows[index];
}

void
jive_graphview_layout(jive_graphview * self)
{
	jive_reservationtracker reservation;
	jive_reservationtracker_init(&reservation, self->graph->context);
	
	/* compute sizes of regions and nodes */
	jive_regionview * regionview = jive_regionview_create(self, self->graph->root_region);
	jive_regionview_layout(regionview, &reservation);
	self->root_region = regionview;
	
	self->width = reservation.max_x - reservation.min_x;
	jive_regionview_move_horizontal(regionview, reservation.min_x);
	
	/* compute sizes of rows and vertical positions, as
	well as total size of graph */
	size_t n;
	for(n=0; n<self->nrows; n++) {
		jive_graphview_row * row = &self->rows[n];
		row->x = 0;
		row->y = self->height + row->pad_above;
		self->height += row->height + row->pad_above + row->pad_below;
	}
	
	jive_reservationtracker_fini(&reservation);
}

void
jive_graphview_draw(jive_graphview * self)
{
	jive_graphview_layout(self);
	jive_textcanvas_init(&self->canvas, self->width, self->height);
	
	jive_regionview_draw(self->root_region, &self->canvas);
	
	size_t n;
	for(n=0; n<self->inputmap.nbuckets; n++) {
		struct jive_inputview_map_entry * entry = self->inputmap.buckets[n];
		while(entry) {
			jive_inputview * inputview = entry->value;
			jive_outputview * outputview = jive_outputview_map_lookup(&self->outputmap, inputview->input->origin)->value;
			
			int begin_x = outputview->nodeview->x + jive_outputview_get_edge_offset(outputview);
			int begin_y = jive_nodeview_get_y(outputview->nodeview) + outputview->nodeview->height - 1;
			int bend_y = jive_nodeview_get_y(inputview->nodeview) + inputview->edge_bend_y;
			int end_x = inputview->nodeview->x + jive_inputview_get_edge_offset(inputview);
			int end_y = jive_nodeview_get_y(inputview->nodeview);
			
			jive_textcanvas_vline(&self->canvas, begin_x, begin_y, bend_y, false, false);
			jive_textcanvas_hline(&self->canvas, begin_x, bend_y, end_x, false, false);
			jive_textcanvas_vline(&self->canvas, end_x, bend_y, end_y, false, false);
			
			entry = entry->next;
		}
	}
}
