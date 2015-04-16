/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/util/textcanvas.h>
#include <jive/view.h>
#include <jive/view/graphview.h>
#include <jive/vsdg/region.h>
#include <jive/util/textcanvas.h>

void
jive_view(const struct jive_graph * graph, FILE * out)
{
	jive_graphview graphview(graph);
	jive_graphview_draw(&graphview);
	jive_textcanvas_write(&graphview.canvas, out);
	fflush(out);
}

std::vector<wchar_t>
jive_view_wstring(const struct jive_graph * graph)
{
	jive_graphview graphview(graph);
	jive_graphview_draw(&graphview);
	return jive_textcanvas_as_wstring(&graphview.canvas);
}

std::string
jive_view_string(const struct jive_graph * graph)
{
	jive_graphview graphview(graph);
	jive_graphview_draw(&graphview);
	return jive_textcanvas_as_string(&graphview.canvas);
}

std::string
jive_view_utf8(const struct jive_graph * graph)
{
	jive_graphview graphview(graph);
	jive_graphview_draw(&graphview);
	return jive_textcanvas_as_utf8(&graphview.canvas);
}

namespace jive {
namespace view {

static std::string
region_tree_string_recursive(const jive_region * region, size_t depth)
{
	std::string string(depth, '-');
	if (region->anchor) {
		char tmp[32];
		snprintf(tmp, sizeof(tmp), "%p", region);
		string.append(region->anchor->node()->operation().debug_string())
		.append("_").append(tmp).append("\n");
	} else
		string.append("ROOT\n");

	jive_region * subregion;
	JIVE_LIST_ITERATE(region->subregions, subregion, region_subregions_list)
		string.append(region_tree_string_recursive(subregion, depth+1));

	return string;
}

std::string
region_tree_string(const jive_region * region)
{
	std::string string;
	string.append(region_tree_string_recursive(region, 0));
	return string;
}

void
region_tree(const jive_region * region, FILE * out)
{
	std::string s = region_tree_string(region);
	fputs(s.c_str(), out);
	fflush(out);
}

}
}
