/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/util/textcanvas.h>
#include <jive/view.h>
#include <jive/view/graphview.h>
#include <jive/util/textcanvas.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/structural_node.h>

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

static std::string
region_to_string(
	const jive::region * region,
	size_t depth,
	std::unordered_map<oport*, std::string> &);

static inline std::string
indent(size_t depth)
{
	return std::string(depth*2, ' ');
}

static inline std::string
create_port_name(const jive::oport * port, std::unordered_map<oport*, std::string> & map)
{
	std::string name = dynamic_cast<const jive::argument*>(port) ? "a" : "o";
	name += jive::detail::strfmt(map.size());
	return name;
}

static inline std::string
node_to_string(
	const jive::node * node,
	size_t depth,
	std::unordered_map<oport*, std::string> & map)
{
	std::string s(indent(depth));
	for (size_t n = 0; n < node->noutputs(); n++) {
		auto name = create_port_name(node->output(n), map);
		map[node->output(n)] = name;
		s = s + name + " ";
	}

	s += ":= " + node->operation().debug_string() + " ";

	for (size_t n = 0; n < node->ninputs(); n++) {
		s += map[node->input(n)->origin()];
		if (n <= node->ninputs()-1)
			s += " ";
	}
	s += "\n";

	if (auto snode = dynamic_cast<const jive::structural_node*>(node)) {
		for (size_t n = 0; n < snode->nsubregions(); n++)
			s += region_to_string(snode->subregion(n), depth+1, map);
	}

	return s;
}

static inline std::string
region_header(const jive::region * region, std::unordered_map<oport*, std::string> & map)
{
	std::string header("[");
	for (size_t n = 0; n < region->narguments(); n++) {
		auto argument = region->argument(n);
		auto pname = create_port_name(argument, map);
		map[argument] = pname;

		header += pname;
		if (argument->input())
			header += detail::strfmt(" <= ", map[argument->input()->origin()]);

		if (n < region->narguments()-1)
			header += ", ";
	}
	header += "]{";

	return header;
}

static inline std::string
region_body(
	const jive::region * region,
	size_t depth,
	std::unordered_map<oport*, std::string> & map)
{
	std::vector<std::vector<const jive::node*>> context;
	for (const auto & node : region->nodes) {
		if (node.depth() >= context.size())
			context.resize(node.depth()+1);
		context[node.depth()].push_back(&node);
	}

	std::string body;
	for (const auto & nodes : context) {
		for (const auto & node : nodes)
			body += node_to_string(node, depth, map);
	}

	return body;
}

static inline std::string
region_footer(const jive::region * region, std::unordered_map<oport*, std::string> & map)
{
	std::string footer("}[");
	for (size_t n = 0; n < region->nresults(); n++) {
		auto result = region->result(n);
		auto pname = map[result->origin()];

		if (result->output())
			footer += map[result->output()] + " <= ";
		footer += pname;

		if (n < region->nresults()-1)
			footer += ", ";
	}
	footer += "]";

	return footer;
}

static inline std::string
region_to_string(
	const jive::region * region,
	size_t depth,
	std::unordered_map<oport*, std::string> & map)
{
	std::string s;
	s = indent(depth) + region_header(region, map) + "\n";
	s = s + region_body(region, depth+1, map);
	s = s + indent(depth) + region_footer(region, map) + "\n";
	return s;
}

std::string
view(const jive::region * region)
{
	std::unordered_map<oport*, std::string> map;
	return region_to_string(region, 0, map);
}

void
view(const jive::region * region, FILE * out)
{
	fputs(view(region).c_str(), out);
	fflush(out);
}

static std::string
region_tree_string_recursive(const jive::region * region, size_t depth)
{
	std::string string(depth, '-');
	if (region->anchor()) {
		char tmp[32];
		snprintf(tmp, sizeof(tmp), "%p", region);
		string.append(region->anchor()->node()->operation().debug_string())
		.append("_").append(tmp).append("\n");
	} else
		string.append("ROOT\n");

	jive::region * subregion;
	JIVE_LIST_ITERATE(region->subregions, subregion, region_subregions_list)
		string.append(region_tree_string_recursive(subregion, depth+1));

	return string;
}

std::string
region_tree_string(const jive::region * region)
{
	std::string string;
	string.append(region_tree_string_recursive(region, 0));
	return string;
}

void
region_tree(const jive::region * region, FILE * out)
{
	std::string s = region_tree_string(region);
	fputs(s.c_str(), out);
	fflush(out);
}

}
