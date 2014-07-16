/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VIEW_NODEVIEW_H
#define JIVE_VIEW_NODEVIEW_H

#include <jive/vsdg/node.h>
#include <jive/vsdg/basetype.h>
#include <jive/util/textcanvas.h>

#include <string>
#include <vector>

class jive_nodeview;
class jive_graphview;
class jive_reservationtracker;

class jive_inputview {
public:
	~jive_inputview() noexcept {}

	jive_inputview(jive_nodeview * nodeview, jive::input * input);

	jive::input * input;
	jive_nodeview * nodeview;
	int x, y, width, height;
	std::string label;
	int edge_bend_y;
	
	struct {
		jive_inputview * prev;
		jive_inputview * next;
	} hash_chain;
};

void
jive_inputview_draw(jive_inputview * self, jive_textcanvas * dst, int x, int y);

static inline int
jive_inputview_get_edge_offset(const jive_inputview * self)
{
	return self->x + (self->width >> 1);
}

class jive_outputview {
public:
	~jive_outputview() noexcept {}

	jive_outputview(jive_nodeview * nodeview, jive::output * output);

	jive::output * output;
	jive_nodeview * nodeview;
	short x, y;
	unsigned short width, height;
	std::string label;
	
	short edge_begin_x, edge_begin_y;
	
	struct {
		jive_outputview * prev;
		jive_outputview * next;
	} hash_chain;
};

void
jive_outputview_draw(jive_outputview * self, jive_textcanvas * dst, int x, int y);

static inline int
jive_outputview_get_edge_offset(const jive_outputview * self)
{
	return self->x + (self->width >> 1);
}

class jive_nodeview {
public:
	~jive_nodeview() noexcept;

	jive_nodeview(jive_graphview * graphview, jive_node * node);

	jive_node * node;
	struct jive_graphview * graphview;
	unsigned int column, row;
	std::vector<jive_inputview*> inputs;
	std::vector<jive_outputview*> outputs;
	bool placed;
	std::string node_label;
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

void
jive_nodeview_layout(jive_nodeview * self, jive_reservationtracker * reservation);

void
jive_nodeview_draw(jive_nodeview * self, jive_textcanvas * dst);

int
jive_nodeview_get_y(const jive_nodeview * self);

#endif
