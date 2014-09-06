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

#include <algorithm>
#include <deque>
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

cfg::cfg()
	: clg_node(nullptr)
{
	create_enter_node();
	create_exit_node();
	enter_->add_outedge(exit_, 0);
}

cfg::cfg(jive::frontend::clg_node & _clg_node)
	: clg_node(&_clg_node)
{
	create_enter_node();
	create_exit_node();
	enter_->add_outedge(exit_, 0);
}

cfg::cfg(const cfg & c)
	: clg_node(nullptr)
{
	std::unordered_map<cfg_node*,cfg_node*> node_map;
	std::unordered_set<std::unique_ptr<cfg_node>>::const_iterator it;
	for (it = c.nodes_.begin(); it != c.nodes_.end(); it++) {
		cfg_node * copy;
		cfg_node * node = (*it).get();
		if (node == c.enter()) {
			create_enter_node();
			copy = enter_;
		} else if (node == c.exit()) {
			create_exit_node();
			copy = exit_;
		} else
			copy = create_basic_block();

		node_map[node] = copy;
	}

	for (it = c.nodes_.begin(); it != c.nodes_.end(); it++) {
		cfg_node * node = (*it).get();
		cfg_node * copy = node_map[node];
		std::vector<cfg_edge*> edges = node->outedges();
		for (auto edge : edges)
			copy->add_outedge(node_map[edge->sink()], edge->index());
	}
}

void
cfg::create_enter_node()
{
	std::unique_ptr<cfg_node> enter(new enter_node(*this));
	enter_ = static_cast<enter_node*>(enter.get());
	nodes_.insert(std::move(enter));
	enter.release();
}

void
cfg::create_exit_node()
{
	std::unique_ptr<cfg_node> exit(new exit_node(*this));
	exit_ = static_cast<exit_node*>(exit.get());
	nodes_.insert(std::move(exit));
	exit.release();
}

jive::frontend::basic_block *
cfg::create_basic_block()
{
	std::unique_ptr<cfg_node> bb(new basic_block(*this));
	basic_block * tmp = static_cast<basic_block*>(bb.get());
	nodes_.insert(std::move(bb));
	bb.release();
	return tmp;
}

std::vector<std::unordered_set<cfg_node*>>
cfg::find_sccs() const
{
	JIVE_DEBUG_ASSERT(is_closed());

	std::vector<std::unordered_set<jive::frontend::cfg_node*>> sccs;

	std::unordered_map<jive::frontend::cfg_node*, std::pair<size_t,size_t>> map;
	std::vector<jive::frontend::cfg_node*> node_stack;
	size_t index = 0;

	std::unordered_set<std::unique_ptr<cfg_node>>::const_iterator it;
	for (it = nodes_.begin(); it != nodes_.end(); it++) {
		if (map.find((*it).get()) == map.end())
			strongconnect((*it).get(), map, node_stack, index, sccs);
	}

	return sccs;
}

bool
cfg::is_closed() const noexcept
{
	JIVE_DEBUG_ASSERT(is_valid());

	std::unordered_set<std::unique_ptr<cfg_node>>::const_iterator it;
	for (it = nodes_.begin(); it != nodes_.end(); it++) {
		jive::frontend::cfg_node * node = (*it).get();
		if (node == enter())
			continue;

		if (node->no_predecessor())
			return false;
	}

	return true;
}

bool
cfg::is_linear() const noexcept
{
	JIVE_DEBUG_ASSERT(is_closed());

	std::unordered_set<std::unique_ptr<cfg_node>>::const_iterator it;
	for (it = nodes_.begin(); it != nodes_.end(); it++) {
		jive::frontend::cfg_node * node = (*it).get();
		if (node == enter() || node == exit())
			continue;

		if (!node->single_successor() || !node->single_predecessor())
			return false;
	}

	return true;
}

bool
cfg::is_structured() const
{
	JIVE_DEBUG_ASSERT(is_closed());

	cfg c(*this);
	std::unordered_set<std::unique_ptr<cfg_node>>::const_iterator it = c.nodes_.begin();
	while (it != c.nodes_.end()) {
		jive::frontend::cfg_node * node = (*it).get();

		if (c.nodes_.size() == 2) {
			JIVE_DEBUG_ASSERT(c.is_closed());
			return true;
		}

		if (node == c.enter() || node == c.exit()) {
			it++; continue;
		}

		/* loop */
		bool is_selfloop = false;
		std::vector<cfg_edge*> edges = node->outedges();
		for (auto edge : edges) {
			if (edge->is_selfloop())  {
				node->remove_outedge(edge);
				is_selfloop = true; break;
			}
		}
		if (is_selfloop) {
			it = c.nodes_.begin(); continue;
		}

		/* linear */
		if (node->single_successor() && node->outedges()[0]->sink()->single_predecessor()) {
			JIVE_DEBUG_ASSERT(node->noutedges() == 1 && node->outedges()[0]->sink()->ninedges() == 1);
			node->divert_inedges(node->outedges()[0]->sink());
			c.remove_node(node);
			it = c.nodes_.begin(); continue;
		}

		/* branch */
		if (node->is_branch()) {
			/* find tail node */
			JIVE_DEBUG_ASSERT(node->noutedges() > 1);
			std::vector<cfg_edge*> edges = node->outedges();
			cfg_node * succ1 = edges[0]->sink();
			cfg_node * succ2 = edges[1]->sink();
			cfg_node * tail = nullptr;
			if (succ1->noutedges() == 1 && succ1->outedges()[0]->sink() == succ2)
				tail = succ2;
			else if (succ2->noutedges() == 1 && succ2->outedges()[0]->sink() == succ1)
				tail = succ1;
			else if (succ1->noutedges() == 1 && succ2->noutedges() == 1
				&& succ1->outedges()[0]->sink() == succ2->outedges()[0]->sink())
				tail = succ1->outedges()[0]->sink();

			if (tail == nullptr || tail->ninedges() != node->noutedges()) {
				it++; continue;
			}

			/* check whether it corresponds to a branch subgraph */
			bool is_branch = true;
			for (auto edge : edges) {
				cfg_node * succ = edge->sink();
				if (succ != tail
				&& ((succ->ninedges() != 1 || succ->inedges()[0]->source() != node)
					|| (succ->noutedges() != 1 || succ->outedges()[0]->sink() != tail))) {
					is_branch = false; break;
				}
			}
			if (!is_branch) {
				it++; continue;
			}

			/* remove branch subgraph */
			for (auto edge : edges) {
				if (edge->sink() != tail)
					c.remove_node(edge->sink());
			}
			node->remove_outedges();
			node->add_outedge(tail, 0);
			it = c.nodes_.begin(); continue;
		}

		it++;
	}

	return false;
}

bool
cfg::is_reducible() const
{
	JIVE_DEBUG_ASSERT(is_closed());

	cfg c(*this);
	std::unordered_set<std::unique_ptr<cfg_node>>::const_iterator it = c.nodes_.begin();
	while (it != c.nodes_.end()) {
		jive::frontend::cfg_node * node = (*it).get();

		if (c.nodes_.size() == 2) {
			JIVE_DEBUG_ASSERT(c.is_closed());
			return true;
		}

		if (node == c.enter() || node == c.exit()) {
			it++; continue;
		}

		/* T1 */
		bool is_selfloop = false;
		std::vector<cfg_edge*> edges = node->outedges();
		for (auto edge : edges) {
			if (edge->is_selfloop()) {
				node->remove_outedge(edge);
				is_selfloop = true; break;
			}
		}
		if (is_selfloop) {
			it = c.nodes_.begin(); continue;
		}

		/* T2 */
		if (node->single_predecessor()) {
			cfg_node * predecessor = node->inedges()[0]->source();
			std::vector<cfg_edge*> edges = node->outedges();
			for (size_t e = 0; e < edges.size(); e++) {
				predecessor->add_outedge(edges[e]->sink(), 0);
			}
			c.remove_node(node);
			it = c.nodes_.begin(); continue;
		}

		it++;
	}

	return false;
}

bool
cfg::is_valid() const
{
	std::unordered_set<std::unique_ptr<cfg_node>>::const_iterator it;
	for (it = nodes_.begin(); it != nodes_.end(); it++) {
		jive::frontend::cfg_node * node = (*it).get();
		if (node == exit()) {
			if (!node->no_successor())
				return false;
			continue;
		}

		if (node == enter()) {
			if (!node->no_predecessor())
				return false;
			if (!node->single_successor())
				return false;
			if (node->outedges()[0]->index() != 0)
				return false;
			continue;
		}

		if (node->no_successor())
			return false;

		/*
			Check whether all indices are 0 and in ascending order (uniqueness of indices)
		*/
		std::vector<jive::frontend::cfg_edge*> edges = node->outedges();
		std::sort(edges.begin(), edges.end(),
			[](const jive::frontend::cfg_edge * e1, const jive::frontend::cfg_edge * e2)
			{ return e1->index() < e2->index(); });
		for (size_t n = 0; n < edges.size(); n++) {
			if (edges[n]->index() != n)
				return false;
		}

		/*
			Check whether the CFG is actually a graph and not a multigraph
		*/
		std::sort(edges.begin(), edges.end(),
			[](const jive::frontend::cfg_edge * e1, const jive::frontend::cfg_edge * e2)
			{ return e1->sink() < e2->sink(); });
		for (size_t n = 1; n < edges.size(); n++) {
			if (edges[n-1]->sink() == edges[n]->sink())
				return false;
		}
	}

	return true;
}

void
cfg::convert_to_dot(jive::buffer & buffer) const
{
	buffer.append("digraph cfg {\n");

	char tmp[96];
	std::unordered_set<std::unique_ptr<cfg_node>>::const_iterator it;
	for (it = nodes_.begin(); it != nodes_.end(); it++) {
		jive::frontend::cfg_node * node = (*it).get();
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
cfg::remove_node(cfg_node * node)
{
	node->remove_inedges();
	node->remove_outedges();

	std::unique_ptr<cfg_node> tmp(node);
	std::unordered_set<std::unique_ptr<cfg_node>>::const_iterator it = nodes_.find(tmp);
	JIVE_DEBUG_ASSERT(it != nodes_.end());
	nodes_.erase(it);
	tmp.release();
}

void
cfg::prune()
{
	JIVE_DEBUG_ASSERT(is_valid());

	/* find all nodes that are dominated by the entry node */
	std::unordered_set<cfg_node*> to_visit({enter_});
	std::unordered_set<cfg_node*> visited;
	while (!to_visit.empty()) {
		cfg_node * node = *to_visit.begin();
		to_visit.erase(to_visit.begin());
		JIVE_DEBUG_ASSERT(visited.find(node) == visited.end());
		visited.insert(node);
		std::vector<cfg_edge*> edges = node->outedges();
		for (auto edge : edges) {
			if (visited.find(edge->sink()) == visited.end()
			&& to_visit.find(edge->sink()) == to_visit.end())
				to_visit.insert(edge->sink());
		}
	}

	/* remove all nodes not dominated by the entry node */
	std::unordered_set<std::unique_ptr<cfg_node>>::iterator it = nodes_.begin();
	while (it != nodes_.end()) {
		if (visited.find((*it).get()) == visited.end()) {
			cfg_node * node = (*it).get();
			node->remove_inedges();
			node->remove_outedges();
			it = nodes_.erase(it);
		} else
			it++;
	}

	JIVE_DEBUG_ASSERT(is_closed());
}

}
}

void
jive_cfg_view(const jive::frontend::cfg & self)
{
	jive::buffer buffer;
	FILE * file = popen("tee /tmp/cfg.dot | dot -Tps > /tmp/cfg.ps ; gv /tmp/cfg.ps", "w");
	self.convert_to_dot(buffer);
	fwrite(buffer.c_str(), buffer.size(), 1, file);
	pclose(file);
}
