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

jive_cfg_node::~jive_cfg_node() noexcept
{
	JIVE_LIST_REMOVE(cfg_->nodes, this, cfg_node_list);
}

jive_cfg_node::jive_cfg_node(struct jive_cfg * cfg) noexcept
	: cfg_(cfg)
	, taken_successor(nullptr)
	, nottaken_successor(nullptr)
{
	taken_predecessors.first = 0;
	taken_predecessors.last = 0;
	taken_predecessors_list.prev = 0;
	taken_predecessors_list.next = 0;

	nottaken_predecessors.first = 0;
	nottaken_predecessors.last = 0;
	nottaken_predecessors_list.prev = 0;
	nottaken_predecessors_list.next = 0;

	cfg_node_list.prev = 0;
	cfg_node_list.next = 0;

	JIVE_LIST_PUSH_BACK(cfg_->nodes, this, cfg_node_list);
}

void
jive_cfg_node::remove_taken_successor() noexcept
{
	if (taken_successor == nullptr)
		return;

	JIVE_LIST_REMOVE(taken_successor->taken_predecessors, this, taken_predecessors_list);
	taken_successor = nullptr;
}

void
jive_cfg_node::remove_nottaken_successor() noexcept
{
	if (nottaken_successor == nullptr)
		return;

	JIVE_LIST_REMOVE(nottaken_successor->nottaken_predecessors, this, nottaken_predecessors_list);
	nottaken_successor = nullptr;
}

void
jive_cfg_node::remove_taken_predecessors() noexcept
{
	jive_cfg_node * pred, * next;
	JIVE_LIST_ITERATE_SAFE(taken_predecessors, pred, next, taken_predecessors_list)
		pred->remove_taken_successor();
}

void
jive_cfg_node::remove_nottaken_predecessors() noexcept
{
	jive_cfg_node * pred, * next;
	JIVE_LIST_ITERATE_SAFE(nottaken_predecessors, pred, next, nottaken_predecessors_list)
		pred->remove_nottaken_successor();
}

void
jive_cfg_node::add_taken_successor(jive_cfg_node * successor) noexcept
{
	JIVE_ASSERT(taken_successor == nullptr);

	JIVE_LIST_PUSH_BACK(successor->taken_predecessors, this, taken_predecessors_list);
	taken_successor = successor;
}

void
jive_cfg_node::add_nottaken_successor(jive_cfg_node * successor) noexcept
{
	JIVE_ASSERT(nottaken_successor == nullptr);

	JIVE_LIST_PUSH_BACK(successor->nottaken_predecessors, this, nottaken_predecessors_list);
	nottaken_successor = successor;

}

void
jive_cfg_node::divert_taken_predecessors(jive_cfg_node * node) noexcept
{
	jive_cfg_node * pred, * next;
	JIVE_LIST_ITERATE_SAFE(taken_predecessors, pred, next, taken_predecessors_list)
		pred->divert_taken_successor(node);
}

void
jive_cfg_node::divert_nottaken_predecessors(jive_cfg_node * node) noexcept
{
	jive_cfg_node * pred, * next;
	JIVE_LIST_ITERATE_SAFE(nottaken_predecessors, pred, next, nottaken_predecessors_list)
		pred->divert_nottaken_successor(node);
}

void
jive_cfg_node_destroy(struct jive_cfg_node * self)
{
	delete self;
}
