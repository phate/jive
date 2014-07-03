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
#include <jive/frontend/cfg_node-private.h>
#include <jive/frontend/tac/three_address_code.h>
#include <jive/util/buffer.h>
#include <jive/util/list.h>

#include <stdlib.h>
#include <stdio.h>

/* cfg enter node */

jive_cfg_enter_node::~jive_cfg_enter_node() noexcept {}

const struct jive_cfg_node_class JIVE_CFG_ENTER_NODE = {
	parent : &JIVE_CFG_NODE,
	name : "ENTER",
	fini : nullptr, /* inherit */
	get_label : jive_cfg_node_get_label_, /* inherit */
};

static struct jive_cfg_node *
jive_cfg_enter_node_create(struct jive_cfg * cfg)
{
	jive_cfg_node * node = new jive_cfg_enter_node;
	node->class_ = &JIVE_CFG_ENTER_NODE;
	jive_cfg_node_init_(node, cfg);
	return node;
}

/* cfg exit node */

jive_cfg_exit_node::~jive_cfg_exit_node() noexcept {}

const jive_cfg_node_class JIVE_CFG_EXIT_NODE = {
	parent : &JIVE_CFG_NODE,
	name : "EXIT",
	fini : nullptr, /* inherit */
	get_label : jive_cfg_node_get_label_, /* inherit */
};

static struct jive_cfg_node *
jive_cfg_exit_node_create(struct jive_cfg * cfg)
{
	jive_cfg_node * node = new jive_cfg_exit_node;
	node->class_ = &JIVE_CFG_EXIT_NODE;
	jive_cfg_node_init_(node, cfg);
	return node;
}

/* cfg */

static void
jive_cfg_init_(struct jive_cfg * self, struct jive_clg_node * clg_node)
{
	self->clg_node = clg_node;
	self->context = clg_node->clg->context;
	self->nodes.first = 0;
	self->nodes.last = 0;

	self->enter = jive_cfg_enter_node_create(self);
	self->exit = jive_cfg_exit_node_create(self);
	jive_cfg_node_connect_nottaken_successor(self->enter, self->exit);

	clg_node->cfg = self;
}

static void
jive_cfg_fini_(struct jive_cfg * self)
{
	while (self->nodes.first) {
		jive_cfg_node_disconnect_predecessors(self->nodes.first);
		jive_cfg_node_disconnect_taken_successor(self->nodes.first);
		jive_cfg_node_disconnect_nottaken_successor(self->nodes.first);
		jive_cfg_node_destroy(self->nodes.first);
	}
}

struct jive_cfg *
jive_cfg_create(struct jive_clg_node * clg_node)
{
	jive_cfg * cfg = jive_context_malloc(clg_node->clg->context, sizeof(*cfg));
	jive_cfg_init_(cfg, clg_node);
	return cfg;
}

bool
jive_cfg_is_empty(const struct jive_cfg * self)
{
	if (self->nodes.first->cfg_node_list.next != self->nodes.last)
		return false;

	jive_cfg_node * enter = self->nodes.first;
	jive_cfg_node * exit = self->nodes.last;
	if (enter != self->enter) {
		enter = self->nodes.last;
		exit = self->nodes.first;
	}

	if (enter != self->enter || exit != self->exit)
		return false;

	return true;
}

void
jive_cfg_convert_dot(const struct jive_cfg * self, struct jive_buffer * buffer)
{
	jive_buffer_putstr(buffer, "digraph cfg {\n");

	char tmp[96];
	jive_cfg_node * node;
	JIVE_LIST_ITERATE(self->nodes, node, cfg_node_list) {
		snprintf(tmp, sizeof(tmp), "%zu", (size_t)node);
		jive_buffer_putstr(buffer, tmp);
		jive_buffer_putstr(buffer, "[shape = box, label = \"");
		jive_cfg_node_get_label(node, buffer);
		jive_buffer_putstr(buffer, "\"];\n");

		if (node->taken_successor) {
			snprintf(tmp, sizeof(tmp), "%zu -> %zu[label = \"t\"];\n", (size_t)node,
				(size_t)node->taken_successor);
			jive_buffer_putstr(buffer, tmp);
		}

		if (node->nottaken_successor) {
			snprintf(tmp, sizeof(tmp), "%zu -> %zu[label = \"nt\"];\n", (size_t)node,
				(size_t)node->nottaken_successor);
			jive_buffer_putstr(buffer, tmp);
		}
	}

	jive_buffer_putstr(buffer, "}\n");
}

void
jive_cfg_view(const struct jive_cfg * self)
{
	jive_buffer buffer;
	jive_buffer_init(&buffer, self->clg_node->clg->context);

	FILE * file = popen("tee /tmp/cfg.dot | dot -Tps > /tmp/cfg.ps ; gv /tmp/cfg.ps", "w");
	jive_cfg_convert_dot(self, &buffer);
	fwrite(buffer.data, buffer.size, 1, file);
	pclose(file);

	jive_buffer_fini(&buffer);
}

void
jive_cfg_destroy(struct jive_cfg * self)
{
	jive_cfg_fini_(self);
	jive_context_free(self->clg_node->clg->context, self);
}

void
jive_cfg_validate(const struct jive_cfg * self)
{
	jive_cfg_node * node;
	JIVE_LIST_ITERATE(self->nodes, node, cfg_node_list) {
		if (dynamic_cast<jive_cfg_enter_node*>(node)) {
			JIVE_ASSERT(node->taken_predecessors.first == NULL);
			JIVE_ASSERT(node->taken_predecessors.last == NULL);
			JIVE_ASSERT(node->nottaken_predecessors.first == NULL);
			JIVE_ASSERT(node->nottaken_predecessors.last == NULL);
			JIVE_ASSERT(node->taken_successor == NULL);
			JIVE_ASSERT(node->nottaken_successor != NULL);

			jive_cfg_node * predecessor;
			jive_cfg_node * successor = node->nottaken_successor;
			JIVE_LIST_ITERATE(successor->nottaken_predecessors, predecessor, nottaken_predecessors_list) {
				if (predecessor == node)
					break;
			}
			JIVE_ASSERT(predecessor != NULL);
			continue;
		}

		if (dynamic_cast<jive_cfg_exit_node*>(node)) {
			JIVE_ASSERT(node->taken_successor == NULL);
			JIVE_ASSERT(node->nottaken_successor == NULL);
		}

		/* check whether all taken predecessors are really taken successors */
		jive_cfg_node * predecessor;
		JIVE_LIST_ITERATE(node->taken_predecessors, predecessor, taken_predecessors_list)
			JIVE_ASSERT(predecessor->taken_successor == node);

		/* check whether all nottaken predecessors are really nottaken successors */
		JIVE_LIST_ITERATE(node->nottaken_predecessors, predecessor, nottaken_predecessors_list)
			JIVE_ASSERT(predecessor->nottaken_successor == node);

		if (dynamic_cast<jive_cfg_exit_node*>(node))
			continue;

		JIVE_ASSERT(node->nottaken_successor != NULL);

		/* check whether nottaken successor is in the nottaken predecessor list */
		jive_cfg_node * successor = node->nottaken_successor;
		JIVE_LIST_ITERATE(successor->nottaken_predecessors, predecessor, nottaken_predecessors_list) {
			if (predecessor == node)
				break;
		}
		JIVE_ASSERT(predecessor != NULL);

		if (node->taken_successor == NULL)
			continue;

		/* check whether taken successor is in the taken predecessor list */
		successor = node->taken_successor;
		JIVE_LIST_ITERATE(successor->taken_predecessors, predecessor, taken_predecessors_list) {
			if (predecessor == node)
				break;
		}
		JIVE_ASSERT(predecessor != NULL);
	}
}
