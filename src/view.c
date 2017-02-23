/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/view.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/simple_node.h>
#include <jive/vsdg/structural_node.h>

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

std::string
region_tree(const jive::region * region)
{
	std::function<std::string(const jive::region *, size_t)> f = [&] (
		const jive::region * region,
		size_t depth
	) {
		std::string subtree;
		if (region->node()) {
			if (region->node()->nsubregions() != 1) {
				subtree += std::string(depth, '-') + detail::strfmt(region) + "\n";
				depth += 1;
			}
		} else {
			subtree = "ROOT\n";
			depth += 1;
		}

		for (const auto & node : region->nodes) {
			if (auto snode = dynamic_cast<const jive::structural_node*>(&node)) {
				subtree += std::string(depth, '-') + snode->operation().debug_string() + "\n";
				for (size_t n = 0; n < snode->nsubregions(); n++)
					subtree += f(snode->subregion(n), depth+1);
			}
		}

		return subtree;
	};

	return f(region, 0);
}

void
region_tree(const jive::region * region, FILE * out)
{
	fputs(region_tree(region).c_str(), out);
	fflush(out);
}

}
