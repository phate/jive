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

#include <stdlib.h>
#include <stdio.h>

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
		nodes.first->remove_predecessors();
		nodes.first->remove_successors();
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
	enter->add_nottaken_successor(exit);
}

cfg::cfg(jive::frontend::clg_node & _clg_node)
	: clg_node(&_clg_node)
{
	nodes.first = 0;
	nodes.last = 0;

	enter = new enter_node(*this);
	exit = new exit_node(*this);
	enter->add_nottaken_successor(exit);
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

		if (node->taken_successor()) {
			snprintf(tmp, sizeof(tmp), "%zu -> %zu[label = \"t\"];\n", (size_t)node,
				(size_t)node->taken_successor());
			buffer.append(tmp);
		}

		if (node->nottaken_successor()) {
			snprintf(tmp, sizeof(tmp), "%zu -> %zu[label = \"nt\"];\n", (size_t)node,
				(size_t)node->nottaken_successor());
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
			JIVE_ASSERT(node->predecessors().size() == 0);
			JIVE_ASSERT(node->taken_successor() == nullptr);
			JIVE_ASSERT(node->nottaken_successor() != nullptr);
		}

		if (node == self.exit) {
			JIVE_ASSERT(node->taken_successor() == NULL);
			JIVE_ASSERT(node->nottaken_successor() == NULL);
		}

		if (node == self.exit)
			continue;

		JIVE_ASSERT(node->nottaken_successor() != NULL);

		/* check whether successors are in the predecessor list */
		size_t n;
		jive::frontend::cfg_node * successor = node->nottaken_successor();
		std::vector<jive::frontend::cfg_edge*> predecessors = successor->predecessors();
		for (n = 0; n < predecessors.size(); n++) {
			if (predecessors[n]->source() == node)
			 break;
		}
		JIVE_ASSERT(n != predecessors.size());

		if (node->taken_successor() == NULL)
			continue;

		successor = node->taken_successor();
		predecessors = successor->predecessors();
		for (n = 0; n != predecessors.size(); n++) {
			if (predecessors[n]->source() == node)
			 break;
		}
		JIVE_ASSERT(n != predecessors.size());
	}
}
