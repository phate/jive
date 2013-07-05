/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <string.h>
#include <stdio.h>

#include <jive/arch/registers.h>
#include <jive/view/nodeview.h>
#include <jive/view/reservationtracker.h>
#include <jive/view/graphview.h>
#include <jive/vsdg.h>
#include <jive/util/buffer.h>
#include <jive/util/list.h>

jive_inputview *
jive_inputview_create(jive_nodeview * nodeview, jive_input * input)
{
	jive_context * context = input->node->graph->context;
	
	jive_inputview * self = jive_context_malloc(context, sizeof(*self));
	
	self->nodeview = nodeview;
	self->input = input;
	self->label = 0;
	
	jive_buffer type_label_buffer, input_label_buffer;
	jive_buffer_init(&type_label_buffer, context);
	jive_buffer_init(&input_label_buffer, context);
	jive_input_get_label(input, &input_label_buffer);
	jive_type_get_label(jive_input_get_type(input), &type_label_buffer);
	const char * input_label = jive_buffer_to_string(&input_label_buffer);
	const char * type_label = jive_buffer_to_string(&type_label_buffer);
	
	jive_ssavar * ssavar = input->ssavar;
	if (ssavar) {
		const jive_resource_name * resname = jive_variable_get_resource_name(ssavar->variable);
		const jive_resource_class * rescls = jive_variable_get_resource_class(ssavar->variable);
		if (resname)
			self->label = jive_context_strjoin(context, input_label, ":", type_label, ":*", resname->name, NULL);
		else
			self->label = jive_context_strjoin(context, input_label, ":", type_label, ":", rescls->name, NULL);
	} else if (input->required_rescls != &jive_root_resource_class) {
		self->label = jive_context_strjoin(context, input_label, ":", type_label, ":", input->required_rescls->name, NULL);
	} else {
		self->label = jive_context_strjoin(context, input_label, ":", type_label, NULL);
	}
	
	jive_buffer_fini(&input_label_buffer);
	jive_buffer_fini(&type_label_buffer);
	
	self->x = 0;
	self->y = 0;
	self->width = strlen(self->label) + 2;
	self->height = 1;
	
	self->edge_bend_y = 0;
	
	return self;
}

void
jive_inputview_destroy(jive_inputview * self)
{
	jive_context_free(self->input->node->graph->context, self->label);
	jive_context_free(self->input->node->graph->context, self);
}

void
jive_inputview_draw(jive_inputview * self, jive_textcanvas * dst, int x, int y)
{
	jive_textcanvas_put_ascii(dst, x+1, y, self->label);
}

jive_outputview *
jive_outputview_create(jive_nodeview * nodeview, jive_output * output)
{
	jive_context * context = output->node->graph->context;
	
	jive_outputview * self = jive_context_malloc(context, sizeof(*self));
	
	self->nodeview = nodeview;
	self->output = output;
	self->label = 0;
	
	jive_buffer type_label_buffer;
	jive_buffer_init(&type_label_buffer, context);
	char * output_label = jive_output_get_label(output);
	jive_type_get_label(jive_output_get_type(output), &type_label_buffer);
	const char * type_label = jive_buffer_to_string(&type_label_buffer);

	jive_ssavar * ssavar = output->ssavar;
	if (ssavar) {
		const jive_resource_name * resname = jive_variable_get_resource_name(ssavar->variable);
		const jive_resource_class * rescls = jive_variable_get_resource_class(ssavar->variable);
		if (resname)
			self->label = jive_context_strjoin(context, output_label, ":", type_label, ":*", resname->name, NULL);
		else
			self->label = jive_context_strjoin(context, output_label, ":", type_label, ":", rescls->name, NULL);
	} else if (output->required_rescls != &jive_root_resource_class) {
		self->label = jive_context_strjoin(context, output_label, ":", type_label, ":", output->required_rescls->name, NULL);
	} else {
		self->label = jive_context_strjoin(context, output_label, ":", type_label, NULL);
	}
	
	free(output_label);
	jive_buffer_fini(&type_label_buffer);
	
	self->x = 0;
	self->y = 0;
	self->width = strlen(self->label) + 2;
	self->height = 1;
	
	return self;
}

void
jive_outputview_destroy(jive_outputview * self)
{
	jive_context_free(self->output->node->graph->context, self->label);
	jive_context_free(self->output->node->graph->context, self);
}

void
jive_outputview_draw(jive_outputview * self, jive_textcanvas * dst, int x, int y)
{
	jive_textcanvas_put_ascii(dst, x+1, y, self->label);
}

static void
jive_nodeview_init(jive_nodeview * self, struct jive_graphview * graphview, jive_node * node)
{
	jive_context * context = node->graph->context;
	
	self->graphview = graphview;
	self->node = node;
	self->column = 0;
	self->row = 0;
	
	self->regionview_nodes_list.prev = 0;
	self->regionview_nodes_list.next = 0;
	
	size_t n;
	self->inputs = jive_context_malloc(context, sizeof(*self->inputs) * node->ninputs);
	for(n=0; n<node->ninputs; n++)
		self->inputs[n] = jive_inputview_create(self, node->inputs[n]);
	
	self->outputs = jive_context_malloc(context, sizeof(*self->outputs) * node->noutputs);
	for(n=0; n<node->noutputs; n++)
		self->outputs[n] = jive_outputview_create(self, node->outputs[n]);
	
	self->placed = false;
	
	char nodeid[32];
	snprintf(nodeid, sizeof(nodeid), "%zx", (size_t) node);
	
	char * node_label = jive_node_get_label(node);
	self->node_label = jive_context_strjoin(context, node_label, ":", nodeid, NULL);
	free(node_label);
	
	int input_width = -3, output_width = -3;
	int cur_x;
	
	int internal_width = strlen(self->node_label);

	cur_x = 1;
	for(n=0; n<self->node->ninputs; n++) {
		self->inputs[n]->x = cur_x;
		cur_x += self->inputs[n]->width + 1;
		input_width += self->inputs[n]->width + 1;
	}
	if (input_width > internal_width) internal_width = input_width;
	
	cur_x = 1;
	for(n=0; n<self->node->noutputs; n++) {
		self->outputs[n]->x = cur_x;
		cur_x += self->outputs[n]->width + 1;
		output_width += self->outputs[n]->width + 1;
	}
	if (output_width > internal_width) internal_width = output_width;
	
	self->width = internal_width + 4;
	self->height = 7;
}

static void
jive_nodeview_fini(jive_nodeview * self)
{
	jive_context * context = self->node->graph->context;
	size_t n;
	for(n=0; n<self->node->ninputs; n++)
		jive_inputview_destroy(self->inputs[n]);
	jive_context_free(context, self->inputs);
	for(n=0; n<self->node->noutputs; n++)
		jive_outputview_destroy(self->outputs[n]);
	jive_context_free(context, self->outputs);
	jive_context_free(context, self->node_label);
}

jive_nodeview *
jive_nodeview_create(struct jive_graphview * graphview, jive_node * node)
{
	jive_nodeview * nodeview = jive_context_malloc(node->graph->context, sizeof(*nodeview));
	jive_nodeview_init(nodeview, graphview, node);
	return nodeview;
}

void
jive_nodeview_destroy(jive_nodeview * self)
{
	jive_nodeview_fini(self);
	jive_context_free(self->node->graph->context, self);
}

void
jive_nodeview_layout(jive_nodeview * self, struct jive_reservationtracker * reservation)
{
	if (self->placed) return;
	
	jive_graphview * graphview = self->graphview;
	
	int preferred_x = 0, total = 0, count = 0;
	size_t n;
	for(n=0; n<self->node->ninputs; n++) {
		jive_inputview * inputview = self->inputs[n];
		jive_output * origin = self->node->inputs[n]->origin;
		jive_outputview * outputview = jive_outputview_map_lookup(&graphview->outputmap, origin);
		if (!outputview->nodeview->placed) continue;
		
		total += outputview->nodeview->x + jive_outputview_get_edge_offset(outputview) - jive_inputview_get_edge_offset(inputview);
		count ++;
	}
	
	for(n=0; n<self->node->noutputs; n++) {
		jive_outputview * outputview = self->outputs[n];
		jive_input * user = self->node->outputs[n]->users.first;
		JIVE_LIST_ITERATE(self->node->outputs[n]->users, user, output_users_list) {
			jive_inputview * inputview = jive_inputview_map_lookup(&graphview->inputmap, user);
			if (!inputview->nodeview->placed) continue;
			
			total += inputview->nodeview->x + jive_inputview_get_edge_offset(inputview) - jive_outputview_get_edge_offset(outputview);
		}
	}
	
	if (count) preferred_x = total / count;
	
	self->placed = true;
	
	int width = self->width;
	int y = self->node->depth_from_root;
	
	/* extents of node itself */
	jive_reservationrect rects[1 + self->node->noutputs];
	rects[0].x = 0;
	rects[0].y = y;
	rects[0].width = width;
	rects[0].height = 1;
	
	/* for each output, determine extents of longest edge */
	size_t nrects = 1;
	for(n=0; n<self->node->noutputs; n++) {
		jive_outputview * outputview = self->outputs[n];
		jive_input * user = outputview->output->users.first;
		if (!user) continue;
		size_t end_row = y;
		while(user) {
			if (user->node->depth_from_root > end_row) end_row = user->node->depth_from_root;
			user = user->output_users_list.next;
		}
		rects[nrects].x = jive_outputview_get_edge_offset(outputview);
		rects[nrects].y = y;
		rects[nrects].width = 1;
		rects[nrects].height = end_row -y;
		
		nrects ++;
	}
	
	self->x = jive_reservationtracker_find_space(reservation, nrects, rects, preferred_x);
	jive_reservationtracker_reserve_boxes(reservation, nrects, rects, self->x);
	
	jive_graphview_row * row = jive_graphview_get_row(self->graphview, self->node->depth_from_root);
	
	for(n=0; n<self->node->ninputs; n++) {
		jive_inputview * inputview = self->inputs[n];
		row->pad_above ++;
		inputview->edge_bend_y = - row->pad_above;
	}
	
	if (self->height > row->height) row->height = self->height;
}

void
jive_nodeview_draw(jive_nodeview * self, jive_textcanvas * dst)
{
	int x = self->x;
	int y = jive_nodeview_get_y(self);
	jive_textcanvas_hline(dst, x, y, x + self->width-1, true, false);
	jive_textcanvas_hline(dst, x, y + 2 , x + self->width-1, false, false);
	jive_textcanvas_hline(dst, x, y + 4, x + self->width-1, false, false);
	jive_textcanvas_hline(dst, x, y + 6, x + self->width-1, true, false);
	
	jive_textcanvas_vline(dst, x, y, y + self->height-1, true, false);
	jive_textcanvas_vline(dst, x + self->width-1, y, y + self->height-1, true, false);
	
	size_t n;
	for(n=0; n<self->node->ninputs; n++) {
		jive_inputview_draw(self->inputs[n], dst, x + self->inputs[n]->x, y+1);
		jive_textcanvas_vline(dst, x + self->inputs[n]->x - 1, y, y+2, false, false);
	}
	
	for(n=0; n<self->node->noutputs; n++) {
		jive_outputview_draw(self->outputs[n], dst, x + self->outputs[n]->x, y+5);
		jive_textcanvas_vline(dst, x + self->outputs[n]->x - 1, y+4, y+6, false, false);
	}
	
	jive_textcanvas_put_ascii(dst, x+2, y+3, self->node_label);
}

int
jive_nodeview_get_y(const jive_nodeview * self)
{
	return jive_graphview_get_row(self->graphview, self->node->depth_from_root)->y;
}

