/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/view.h>
#include <jive/view/graphview.h>
#include <jive/util/textcanvas.h>

void
jive_view(struct jive_graph * graph, FILE * out)
{
	jive_graphview graphview(graph);
	jive_graphview_draw(&graphview);
	jive_textcanvas_write(&graphview.canvas, out);
	fflush(out);
}

std::vector<wchar_t>
jive_view_wstring(struct jive_graph * graph)
{
	jive_graphview graphview(graph);
	jive_graphview_draw(&graphview);
	return jive_textcanvas_as_wstring(&graphview.canvas);
}

std::string
jive_view_string(struct jive_graph * graph)
{
	jive_graphview graphview(graph);
	jive_graphview_draw(&graphview);
	return jive_textcanvas_as_string(&graphview.canvas);
}

std::string
jive_view_utf8(struct jive_graph * graph)
{
	jive_graphview graphview(graph);
	jive_graphview_draw(&graphview);
	return jive_textcanvas_as_utf8(&graphview.canvas);
}

