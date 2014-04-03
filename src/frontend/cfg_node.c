/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/context.h>
#include <jive/common.h>
#include <jive/frontend/clg.h>
#include <jive/frontend/clg_node.h>
#include <jive/frontend/cfg.h>
#include <jive/frontend/cfg_node.h>
#include <jive/frontend/cfg_node-private.h>
#include <jive/util/buffer.h>
#include <jive/util/list.h>

#include <string.h>

const struct jive_cfg_node_class JIVE_CFG_NODE = {
	parent : 0,
	name : "CFG_NODE",
	fini : jive_cfg_node_fini_,
	get_label : jive_cfg_node_get_label_,
	create : jive_cfg_node_create_
};

void
jive_cfg_node_init_(struct jive_cfg_node * self, struct jive_cfg * cfg)
{
	self->cfg = cfg;

	self->taken_predecessors.first = 0;
	self->taken_predecessors.last = 0;
	self->taken_predecessors_list.prev = 0;
	self->taken_predecessors_list.next = 0;

	self->nottaken_predecessors.first = 0;
	self->nottaken_predecessors.last = 0;
	self->nottaken_predecessors_list.prev = 0;
	self->nottaken_predecessors_list.next = 0;

	self->taken_successor = 0;
	self->nottaken_successor = 0;

	self->cfg_node_list.prev = 0;
	self->cfg_node_list.next = 0;

	JIVE_LIST_PUSH_BACK(cfg->nodes, self, cfg_node_list);
}

void
jive_cfg_node_fini_(struct jive_cfg_node * self)
{
	JIVE_LIST_REMOVE(self->cfg->nodes, self, cfg_node_list);

	self->taken_predecessors.first = 0;
	self->taken_predecessors.last = 0;
	self->taken_predecessors_list.prev = 0;
	self->taken_predecessors_list.next = 0;

	self->nottaken_predecessors.first = 0;
	self->nottaken_predecessors.last = 0;
	self->nottaken_predecessors_list.prev = 0;
	self->nottaken_predecessors_list.next = 0;

	self->taken_successor = 0;
	self->nottaken_successor = 0;
}

void
jive_cfg_node_get_label_(const jive_cfg_node * self, struct jive_buffer * buffer)
{
	jive_buffer_putstr(buffer, self->class_->name);
}

struct jive_cfg_node *
jive_cfg_node_create_(struct jive_cfg * cfg)
{
	jive_cfg_node * node = jive_context_malloc(cfg->context, sizeof(*node));

	node->class_ = &JIVE_CFG_NODE;
	jive_cfg_node_init_(node, cfg);

	return node;
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
	JIVE_ASSERT(self->taken_successor == NULL);
	JIVE_ASSERT(self->taken_predecessors.first == NULL);
	JIVE_ASSERT(self->nottaken_successor == NULL);
	JIVE_ASSERT(self->nottaken_predecessors.first == NULL);

	self->class_->fini(self);
	jive_context_free(self->cfg->context, self);
}
