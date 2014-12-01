/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VIEW_GRAPHVIEW_H
#define JIVE_VIEW_GRAPHVIEW_H

#include <jive/util/textcanvas.h>
#include <jive/view/nodeview.h>
#include <jive/vsdg.h>

#include <unordered_map>
#include <vector>

class jive_regionview;

class jive_graphview_row {
public:
	inline ~jive_graphview_row() noexcept {}

	jive_graphview_row() noexcept;

	int width, height, pad_above, pad_below;
	int x, y;
};

class jive_graphview {
public:
	~jive_graphview() noexcept {}

	jive_graphview(jive_graph * graph);

	struct jive_graph * graph;
	std::unordered_map<jive_node*, jive_nodeview*> nodemap;
	std::unordered_map<jive::input*, jive_inputview*> inputmap;
	std::unordered_map<jive::output*, jive_outputview*> outputmap;
	int width, height;
	
	std::vector<jive_graphview_row> rows;
	
	jive_textcanvas canvas;
};

jive_graphview_row *
jive_graphview_get_row(jive_graphview * self, size_t index);

void
jive_graphview_draw(jive_graphview * self);

#endif
