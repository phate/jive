/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/common.h>
#include <jive/context.h>
#include <jive/frontend/clg.h>
#include <jive/frontend/clg_node.h>
#include <jive/frontend/basic_block.h>
#include <jive/frontend/cfg.h>
#include <jive/frontend/cfg_node.h>
#include <jive/util/buffer.h>
#include <jive/util/list.h>

#include <algorithm>
#include <unordered_map>

#include <stdlib.h>
#include <stdio.h>

/* Tarjan's SCC algorithm */

static void
strongconnect(
	jive::frontend::cfg_node * node,
	std::unordered_map<jive::frontend::cfg_node*, std::pair<size_t,size_t>> & map,
	std::vector<jive::frontend::cfg_node*> & node_stack,
	size_t index,
	std::vector<std::unordered_set<jive::frontend::cfg_node*>> & sccs)
{
	map.emplace(node, std::make_pair(index, index));
	node_stack.push_back(node);
	index++;

	std::vector<jive::frontend::cfg_edge*> edges = node->outedges();
	for (size_t n = 0; n < edges.size(); n++) {
		jive::frontend::cfg_node * successor = edges[n]->sink();
		if (map.find(successor) == map.end()) {
			/* taken successor has not been visited yet; recurse on it */
			strongconnect(successor, map, node_stack, index, sccs);
			map[node].second = std::min(map[node].second, map[successor].second);
		} else if (std::find(node_stack.begin(), node_stack.end(), successor) != node_stack.end()) {
			/* taken successor is in stack and hence in the current SCC */
			map[node].second = std::min(map[node].second, map[successor].first);
		}
	}

	if (map[node].second == map[node].first) {
		std::unordered_set<jive::frontend::cfg_node*> scc;
		jive::frontend::cfg_node * w;
		do {
			w = node_stack.back();
			node_stack.pop_back();
			scc.insert(w);
		} while (w != node);
		sccs.push_back(scc);
	}
}

namespace jive {
namespace frontend {

/* enter node */

cfg::enter_node::~enter_node() noexcept {}

cfg::enter_node::enter_node(jive::frontend::cfg & cfg) noexcept
	: cfg_node(cfg)
{}

std::string
cfg::enter_node::debug_string() const
{
	return std::string("ENTER");
}

/* exit node */

cfg::exit_node::~exit_node() noexcept {}

cfg::exit_node::exit_node(jive::frontend::cfg & cfg) noexcept
	: cfg_node(cfg)
{}

std::string
cfg::exit_node::debug_string() const
{
	return std::string("EXIT");
}


/* cfg */

cfg::~cfg()
{
	while (nodes.first) {
		delete nodes.first;
	}
}

cfg::cfg()
	: clg_node(nullptr)
{
	nodes.first = 0;
	nodes.last = 0;

	enter = new enter_node(*this);
	exit = new exit_node(*this);
	enter->add_outedge(exit, 0);
}

cfg::cfg(jive::frontend::clg_node & _clg_node)
	: clg_node(&_clg_node)
{
	nodes.first = 0;
	nodes.last = 0;

	enter = new enter_node(*this);
	exit = new exit_node(*this);
	enter->add_outedge(exit, 0);
}

std::vector<std::unordered_set<cfg_node*>>
cfg::find_sccs() const
{
	std::vector<std::unordered_set<jive::frontend::cfg_node*>> sccs;

	std::unordered_map<jive::frontend::cfg_node*, std::pair<size_t,size_t>> map;
	std::vector<jive::frontend::cfg_node*> node_stack;
	size_t index = 0;

	jive::frontend::cfg_node * cfg_node;
	JIVE_LIST_ITERATE(nodes, cfg_node, cfg_node_list) {
		if (map.find(cfg_node) == map.end())
			strongconnect(cfg_node, map, node_stack, index, sccs);
	}

	return sccs;
}

}
}

void
jive_cfg_convert_dot(const jive::frontend::cfg & self, jive::buffer & buffer)
{
	buffer.append("digraph cfg {\n");

	char tmp[96];
	jive::frontend::cfg_node * node;
	JIVE_LIST_ITERATE(self.nodes, node, cfg_node_list) {
		snprintf(tmp, sizeof(tmp), "%zu", (size_t)node);
		buffer.append(tmp).append("[shape = box, label = \"");
		buffer.append(node->debug_string().c_str()).append("\"];\n");

		std::vector<jive::frontend::cfg_edge*> edges = node->outedges();
		for (size_t n = 0; n < edges.size(); n++) {
			snprintf(tmp, sizeof(tmp), "%zu -> %zu[label = \"%zu\"];\n", (size_t)edges[n]->source(),
				(size_t)edges[n]->sink(), edges[n]->index());
			buffer.append(tmp);
		}
	}

	buffer.append("}\n");
}

void
jive_cfg_view(const jive::frontend::cfg & self)
{
	jive::buffer buffer;
	FILE * file = popen("tee /tmp/cfg.dot | dot -Tps > /tmp/cfg.ps ; gv /tmp/cfg.ps", "w");
	jive_cfg_convert_dot(self, buffer);
	fwrite(buffer.c_str(), buffer.size(), 1, file);
	pclose(file);
}

void
jive_cfg_validate(const jive::frontend::cfg & self)
{
	jive::frontend::cfg_node * node;
	JIVE_LIST_ITERATE(self.nodes, node, cfg_node_list) {
		if (node == self.enter) {
			JIVE_ASSERT(node->no_predecessor());
			JIVE_ASSERT(node->single_successor());
		}

		if (node == self.exit) {
			JIVE_ASSERT(node->no_successor());
			continue;
		}
	}
}
