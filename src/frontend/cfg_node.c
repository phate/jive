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

jive_cfg_node::~jive_cfg_node()
{
	JIVE_LIST_REMOVE(cfg_->nodes, this, cfg_node_list);
}

jive_cfg_node::jive_cfg_node(struct jive_cfg * cfg)
	: cfg_(cfg)
	, taken_successor_(nullptr)
	, nottaken_successor_(nullptr)
{
	cfg_node_list.prev = 0;
	cfg_node_list.next = 0;

	JIVE_LIST_PUSH_BACK(cfg_->nodes, this, cfg_node_list);
}

void
jive_cfg_node::remove_taken_successor()
{
	if (taken_successor_ == nullptr)
		return;

	taken_successor_->predecessors_.erase(std::remove(taken_successor_->predecessors_.begin(),
		taken_successor_->predecessors_.end(), this), taken_successor_->predecessors_.end());
	taken_successor_ = nullptr;
}

void
jive_cfg_node::remove_nottaken_successor()
{
	if (nottaken_successor_ == nullptr)
		return;

	nottaken_successor_->predecessors_.erase(std::remove(nottaken_successor_->predecessors_.begin(),
		nottaken_successor_->predecessors_.end(), this), nottaken_successor_->predecessors_.end());
	nottaken_successor_ = nullptr;
}

void
jive_cfg_node::remove_predecessors()
{
	for (auto pred : predecessors_) {
		if (pred->taken_successor_ == this)
			pred->taken_successor_ == nullptr;
		if (pred->nottaken_successor_ == this)
			pred->nottaken_successor_ == nullptr;
	}
	predecessors_.clear();
}

void
jive_cfg_node::add_taken_successor(jive_cfg_node * successor)
{
	JIVE_ASSERT(taken_successor_ == nullptr);

	successor->predecessors_.push_back(this);
	taken_successor_ = successor;
}

void
jive_cfg_node::add_nottaken_successor(jive_cfg_node * successor)
{
	JIVE_ASSERT(nottaken_successor_ == nullptr);

	successor->predecessors_.push_back(this);
	nottaken_successor_ = successor;

}

void
jive_cfg_node::divert_predecessors(jive_cfg_node * node)
{
	for (auto pred : predecessors_) {
		if (pred->taken_successor_ == this)
			pred->divert_taken_successor(node);
		if (pred->nottaken_successor_ == this)
			pred->divert_nottaken_successor(node);
	}
}
