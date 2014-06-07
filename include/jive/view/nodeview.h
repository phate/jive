/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VIEW_NODEVIEW_H
#define JIVE_VIEW_NODEVIEW_H

#include <jive/vsdg/node.h>
#include <jive/vsdg/basetype.h>
#include <jive/util/textcanvas.h>

typedef struct jive_inputview jive_inputview;
typedef struct jive_outputview jive_outputview;
typedef struct jive_nodeview jive_nodeview;

struct jive_graphview;
struct jive_reservationtracker;

struct jive_inputview {
	jive::input * input;
	jive_nodeview * nodeview;
	int x, y, width, height;
	char * label;
	int edge_bend_y;
	
	struct {
		jive_inputview * prev;
		jive_inputview * next;
	} hash_chain;
};

jive_inputview *
jive_inputview_create(jive_nodeview * nodeview, jive::input * input);

void
jive_inputview_destroy(jive_inputview * self);

void
jive_inputview_draw(jive_inputview * self, jive_textcanvas * dst, int x, int y);

static inline int
jive_inputview_get_edge_offset(const jive_inputview * self)
{
	return self->x + (self->width >> 1);
}

struct jive_outputview {
	jive_output * output;
	jive_nodeview * nodeview;
	short x, y;
	unsigned short width, height;
	char * label;
	
	short edge_begin_x, edge_begin_y;
	
	struct {
		jive_outputview * prev;
		jive_outputview * next;
	} hash_chain;
};

jive_outputview *
jive_outputview_create(jive_nodeview * nodeview, jive_output * output);

void
jive_outputview_destroy(jive_outputview * self);

void
jive_outputview_draw(jive_outputview * self, jive_textcanvas * dst, int x, int y);

static inline int
jive_outputview_get_edge_offset(const jive_outputview * self)
{
	return self->x + (self->width >> 1);
}

struct jive_nodeview {
	jive_node * node;
	struct jive_graphview * graphview;
	unsigned int column, row;
	jive_inputview ** inputs;
	jive_outputview ** outputs;
	bool placed;
	char * node_label;
	int x, width, height;
	
	struct {
		jive_nodeview * prev;
		jive_nodeview * next;
	} regionview_nodes_list;
	
	struct {
		jive_nodeview * prev;
		jive_nodeview * next;
	} hash_chain;
};

jive_nodeview *
jive_nodeview_create(struct jive_graphview * graphview, jive_node * node);

void
jive_nodeview_destroy(jive_nodeview * self);

void
jive_nodeview_layout(jive_nodeview * self, struct jive_reservationtracker * reservation);

void
jive_nodeview_draw(jive_nodeview * self, jive_textcanvas * dst);

int
jive_nodeview_get_y(const jive_nodeview * self);

#endif
