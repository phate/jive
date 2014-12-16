/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VIEW_H
#define JIVE_VIEW_H

#include <stdio.h>
#include <wchar.h>

#include <string>
#include <vector>

struct jive_graph;
struct jive_region;

void
jive_view(struct jive_graph * graph, FILE * out);

/**
	\brief Return graph represented as unicode string
*/
std::vector<wchar_t>
jive_view_wstring(struct jive_graph * graph);

/**
	\brief Return graph represented as (locale-dependent) string
*/
std::string
jive_view_string(struct jive_graph * graph);

/**
	\brief Return graph represented as utf8 string
*/
std::string
jive_view_utf8(struct jive_graph * graph);

namespace jive {
namespace view {

std::string
region_tree_string(const jive_region * region);

void
region_tree(const jive_region * region, FILE * out);

}
}

#endif
