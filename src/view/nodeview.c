/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <stdio.h>
#include <string.h>

#include <jive/arch/registers.h>
#include <jive/util/buffer.h>
#include <jive/view/graphview.h>
#include <jive/view/nodeview.h>
#include <jive/view/reservationtracker.h>
#include <jive/vsdg.h>

/* inputview */

jive_inputview::jive_inputview(jive_nodeview * nodeview_, jive::input * input_)
	: nodeview(nodeview_)
	, input(input_)
	, x(0)
	, y(0)
	, height(1)
	, edge_bend_y(0)
{
	jive_buffer input_label_buffer;
	input->label(input_label_buffer);
	const char * input_label = jive_buffer_to_string(&input_label_buffer);

	label = std::string(input_label).append(":").append(input->type().debug_string());
	
	jive_ssavar * ssavar = input->ssavar;
	if (ssavar) {
		const jive_resource_name * resname = jive_variable_get_resource_name(ssavar->variable);
		const jive_resource_class * rescls = jive_variable_get_resource_class(ssavar->variable);
		if (resname)
			label.append(":*").append(resname->name);
		else
			label.append(":").append(rescls->name);
	} else if (input->required_rescls != &jive_root_resource_class)
		label.append(":").append(input->required_rescls->name);
	
	width = label.size()+2;
}

void
jive_inputview_draw(jive_inputview * self, jive_textcanvas * dst, int x, int y)
{
	jive_textcanvas_put_ascii(dst, x+1, y, self->label.c_str());
}

/* outputview */

jive_outputview::jive_outputview(jive_nodeview * nodeview_, jive::output * output_)
	: nodeview(nodeview_)
	, output(output_)
	, x(0)
	, y(0)
	, height(1)
	, edge_begin_x(0)
	, edge_begin_y(0)
{
	jive_buffer output_label_buffer;
	output->label(output_label_buffer);
	const char * output_label = jive_buffer_to_string(&output_label_buffer);

	label = std::string(output_label).append(":").append(output->type().debug_string());

	jive_ssavar * ssavar = output->ssavar;
	if (ssavar) {
		const jive_resource_name * resname = jive_variable_get_resource_name(ssavar->variable);
		const jive_resource_class * rescls = jive_variable_get_resource_class(ssavar->variable);
		if (resname)
			label.append(":*").append(resname->name);
		else
			label.append(":").append(rescls->name);
	} else if (output->required_rescls != &jive_root_resource_class)
		label.append(":").append(output->required_rescls->name);

	width = label.size()+2;
}

void
jive_outputview_draw(jive_outputview * self, jive_textcanvas * dst, int x, int y)
{
	jive_textcanvas_put_ascii(dst, x+1, y, self->label.c_str());
}

/* nodeview */

jive_nodeview::jive_nodeview(jive_graphview * graphview_, jive_node * node_)
	: graphview(graphview_)
	, node(node_)
	, column(0)
	, row(0)
	, placed(false)
	, x(0)
	, height(7)
{
	for (size_t n = 0; n < node->ninputs; n++)
		inputs.push_back(jive_inputview(this, node->inputs[n]));
	
	for (size_t n = 0; n < node->noutputs; n++)
		outputs.push_back(jive_outputview(this, node->outputs[n]));
	
	char nodeid[32];
	snprintf(nodeid, sizeof(nodeid), "%zx", (size_t) node);
	
	jive_buffer node_label_buffer;
	jive_node_get_label(node, &node_label_buffer);
	node_label = std::string(jive_buffer_to_string(&node_label_buffer)).append(":").append(nodeid);
	
	int input_width = -3, output_width = -3;
	int cur_x;
	
	int internal_width = node_label.size();

	cur_x = 1;
	for (size_t n = 0; n < node->ninputs; n++) {
		inputs[n].x = cur_x;
		cur_x += inputs[n].width + 1;
		input_width += inputs[n].width + 1;
	}
	if (input_width > internal_width) internal_width = input_width;
	
	cur_x = 1;
	for (size_t n = 0; n < node->noutputs; n++) {
		outputs[n].x = cur_x;
		cur_x += outputs[n].width + 1;
		output_width += outputs[n].width + 1;
	}
	if (output_width > internal_width) internal_width = output_width;
	
	width = internal_width + 4;
}

void
jive_nodeview_layout(jive_nodeview * self, jive_reservationtracker * reservation)
{
	if (self->placed) return;
	
	jive_graphview * graphview = self->graphview;
	
	int preferred_x = 0, total = 0, count = 0;
	size_t n;
	for(n=0; n<self->node->ninputs; n++) {
		jive_inputview * inputview = &self->inputs[n];
		jive::output * origin = self->node->inputs[n]->origin();
		jive_outputview * outputview = graphview->outputmap[origin];
		if (!outputview->nodeview->placed) continue;
		
		total += outputview->nodeview->x + jive_outputview_get_edge_offset(outputview)
			- jive_inputview_get_edge_offset(inputview);
		count ++;
	}
	
	for(n=0; n<self->node->noutputs; n++) {
		jive_outputview * outputview = &self->outputs[n];
		jive::input * user = self->node->outputs[n]->users.first;
		JIVE_LIST_ITERATE(self->node->outputs[n]->users, user, output_users_list) {
			jive_inputview * inputview = graphview->inputmap[user];
			if (!inputview->nodeview->placed) continue;
			
			total += inputview->nodeview->x + jive_inputview_get_edge_offset(inputview)
				- jive_outputview_get_edge_offset(outputview);
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
		jive_outputview * outputview = &self->outputs[n];
		jive::input * user = outputview->output->users.first;
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
		jive_inputview * inputview = &self->inputs[n];
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
		jive_inputview_draw(&self->inputs[n], dst, x + self->inputs[n].x, y+1);
		jive_textcanvas_vline(dst, x + self->inputs[n].x - 1, y, y+2, false, false);
	}
	
	for(n=0; n<self->node->noutputs; n++) {
		jive_outputview_draw(&self->outputs[n], dst, x + self->outputs[n].x, y+5);
		jive_textcanvas_vline(dst, x + self->outputs[n].x - 1, y+4, y+6, false, false);
	}
	
	jive_textcanvas_put_ascii(dst, x+2, y+3, self->node_label.c_str());
}

int
jive_nodeview_get_y(const jive_nodeview * self)
{
	return jive_graphview_get_row(self->graphview, self->node->depth_from_root)->y;
}

