/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/common.h>
#include <jive/frontend/clg.h>
#include <jive/frontend/clg_node.h>
#include <jive/frontend/cfg.h>
#include <jive/frontend/cfg_node.h>
#include <jive/util/list.h>

#include <string.h>

#include <algorithm>

namespace jive {
namespace frontend {

cfg_edge::cfg_edge(cfg_node * source, cfg_node * sink, size_t index) noexcept
	: source_(source)
	, sink_(sink)
	, index_(index)
{}

void
cfg_edge::divert(cfg_node * new_sink, size_t new_index) noexcept
{
	sink_ = new_sink;
	index_ = new_index;
}

cfg_node::~cfg_node()
{
	JIVE_LIST_REMOVE(cfg_->nodes, this, cfg_node_list);
}

cfg_node::cfg_node(jive::frontend::cfg & cfg)
	: cfg_(&cfg)
{
	cfg_node_list.prev = 0;
	cfg_node_list.next = 0;

	JIVE_LIST_PUSH_BACK(cfg_->nodes, this, cfg_node_list);
}

void
cfg_node::remove_taken_successor()
{
	if (taken_edge_ == nullptr)
		return;

	size_t index = taken_edge_->index();
	for (size_t n = index+1; n < taken_successor()->npredecessors(); n++) {
		jive::frontend::cfg_edge * edge = taken_successor()->predecessors_[n];
		edge->divert(edge->sink(), edge->index()-1);
	}

	taken_successor()->predecessors_.erase(taken_successor()->predecessors_.begin()+index);
	taken_edge_.reset();
}

void
cfg_node::remove_nottaken_successor()
{
	if (nottaken_edge_ == nullptr)
		return;

	size_t index = nottaken_edge_->index();
	for (size_t n = index+1; n < nottaken_successor()->npredecessors(); n++) {
		jive::frontend::cfg_edge * edge = nottaken_successor()->predecessors_[n];
		edge->divert(edge->sink(), edge->index()-1);
	}

	nottaken_successor()->predecessors_.erase(nottaken_successor()->predecessors_.begin()+index);
	nottaken_edge_.reset();
}

void
cfg_node::remove_predecessors()
{
	for (auto edge : predecessors_) {
		if (edge->source()->taken_edge_.get() == edge)
			edge->source()->taken_edge_.reset();
		else if (edge->source()->nottaken_edge_.get() == edge)
			edge->source()->nottaken_edge_.reset();
	}
	predecessors_.clear();
}

void
cfg_node::add_taken_successor(jive::frontend::cfg_node * successor)
{
	JIVE_ASSERT(taken_edge_ == nullptr);

	taken_edge_ = std::unique_ptr<jive::frontend::cfg_edge>(
		new jive::frontend::cfg_edge(this, successor, successor->npredecessors()));
	successor->predecessors_.push_back(taken_edge_.get());
}

void
cfg_node::add_nottaken_successor(jive::frontend::cfg_node * successor)
{
	JIVE_ASSERT(nottaken_edge_ == nullptr);

	nottaken_edge_ = std::unique_ptr<jive::frontend::cfg_edge>(
		new jive::frontend::cfg_edge(this, successor, successor->npredecessors()));
	successor->predecessors_.push_back(nottaken_edge_.get());
}

void
cfg_node::divert_predecessors(jive::frontend::cfg_node * node)
{
	for (size_t n = 0; n < npredecessors(); n++) {
		JIVE_DEBUG_ASSERT(predecessors_[n]->sink() == this);
		predecessors_[n]->divert(node, node->npredecessors());
		node->predecessors_.push_back(predecessors_[n]);
	}
	predecessors_.clear();
}

cfg_node *
cfg_node::taken_successor() const noexcept
{
	return taken_edge_ == nullptr ? nullptr : taken_edge_->sink();
}

cfg_node *
cfg_node::nottaken_successor() const noexcept
{
	return nottaken_edge_ == nullptr ? nullptr : nottaken_edge_->sink();
}

}
}
