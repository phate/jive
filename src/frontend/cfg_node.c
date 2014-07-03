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
	JIVE_LIST_REMOVE(cfg->nodes, this, cfg_node_list);
}

jive_cfg_node::jive_cfg_node(struct jive_cfg * cfg_) noexcept
	: cfg(cfg_)
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

	JIVE_LIST_PUSH_BACK(cfg->nodes, this, cfg_node_list);
}

void
jive_cfg_node_connect_taken_successor(struct jive_cfg_node * self, struct jive_cfg_node * successor)
{
	JIVE_ASSERT(self->taken_successor == NULL);

	JIVE_LIST_PUSH_BACK(successor->taken_predecessors, self, taken_predecessors_list);
	self->taken_successor = successor;
}

void
jive_cfg_node_connect_nottaken_successor(struct jive_cfg_node * self, struct jive_cfg_node * successor)
{
	JIVE_ASSERT(self->nottaken_successor == NULL);

	JIVE_LIST_PUSH_BACK(successor->nottaken_predecessors, self, nottaken_predecessors_list);
	self->nottaken_successor = successor;
}

void
jive_cfg_node_disconnect_taken_successor(struct jive_cfg_node * self)
{
	if (self->taken_successor == NULL)
		return;

	JIVE_LIST_REMOVE(self->taken_successor->taken_predecessors, self, taken_predecessors_list);
	self->taken_successor = NULL;
}

void
jive_cfg_node_disconnect_nottaken_successor(struct jive_cfg_node * self)
{
	if (self->nottaken_successor == NULL)
		return;

	JIVE_LIST_REMOVE(self->nottaken_successor->nottaken_predecessors, self,
		nottaken_predecessors_list);
	self->nottaken_successor = NULL;
}

void
jive_cfg_node_disconnect_taken_predecessors(struct jive_cfg_node * self)
{
	jive_cfg_node * pred, * next;
	JIVE_LIST_ITERATE_SAFE(self->taken_predecessors, pred, next, taken_predecessors_list)
		jive_cfg_node_disconnect_taken_successor(pred);
}

void
jive_cfg_node_disconnect_nottaken_predecessors(struct jive_cfg_node * self)
{
	jive_cfg_node * pred, * next;
	JIVE_LIST_ITERATE_SAFE(self->nottaken_predecessors, pred, next, nottaken_predecessors_list)
		jive_cfg_node_disconnect_nottaken_successor(pred);
}

void
jive_cfg_node_divert_taken_predecessors(struct jive_cfg_node * self, struct jive_cfg_node * node)
{
	jive_cfg_node * pred, * next;
	JIVE_LIST_ITERATE_SAFE(self->taken_predecessors, pred, next, taken_predecessors_list)
		jive_cfg_node_divert_taken_successor(pred, node);
}

void
jive_cfg_node_divert_nottaken_predecessors(struct jive_cfg_node * self, struct jive_cfg_node * node)
{
	jive_cfg_node * pred, * next;
	JIVE_LIST_ITERATE_SAFE(self->nottaken_predecessors, pred, next, nottaken_predecessors_list)
		jive_cfg_node_divert_nottaken_successor(pred, node);
}

void
jive_cfg_node_destroy(struct jive_cfg_node * self)
{
	delete self;
}
