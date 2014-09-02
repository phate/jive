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

/* cfg enter node */

jive_cfg_enter_node::~jive_cfg_enter_node() noexcept {}

jive_cfg_enter_node::jive_cfg_enter_node(struct jive_cfg * cfg) noexcept
	: jive_cfg_node(cfg)
{}

std::string
jive_cfg_enter_node::debug_string() const
{
	return std::string("ENTER");
}

static struct jive_cfg_node *
jive_cfg_enter_node_create(struct jive_cfg * cfg)
{
	return new jive_cfg_enter_node(cfg);
}

/* cfg exit node */

jive_cfg_exit_node::~jive_cfg_exit_node() noexcept {}

jive_cfg_exit_node::jive_cfg_exit_node(struct jive_cfg * cfg) noexcept
	: jive_cfg_node(cfg)
{}

std::string
jive_cfg_exit_node::debug_string() const
{
	return std::string("EXIT");
}

static struct jive_cfg_node *
jive_cfg_exit_node_create(struct jive_cfg * cfg)
{
	return new jive_cfg_exit_node(cfg);
}

/* cfg */

static void
jive_cfg_init_(struct jive_cfg * self, struct jive_clg_node * clg_node)
{
	self->clg_node = clg_node;
	self->nodes.first = 0;
	self->nodes.last = 0;

	self->enter = jive_cfg_enter_node_create(self);
	self->exit = jive_cfg_exit_node_create(self);
	self->enter->add_nottaken_successor(self->exit);

	clg_node->cfg = self;
}

static void
jive_cfg_fini_(struct jive_cfg * self)
{
	while (self->nodes.first) {
		self->nodes.first->remove_predecessors();
		self->nodes.first->remove_successors();
		delete self->nodes.first;
	}
}

struct jive_cfg *
jive_cfg_create(struct jive_clg_node * clg_node)
{
	jive_cfg * cfg = new jive_cfg;
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
		jive_buffer_putstr(buffer, node->debug_string().c_str());
		jive_buffer_putstr(buffer, "\"];\n");

		if (node->taken_successor()) {
			snprintf(tmp, sizeof(tmp), "%zu -> %zu[label = \"t\"];\n", (size_t)node,
				(size_t)node->taken_successor());
			jive_buffer_putstr(buffer, tmp);
		}

		if (node->nottaken_successor()) {
			snprintf(tmp, sizeof(tmp), "%zu -> %zu[label = \"nt\"];\n", (size_t)node,
				(size_t)node->nottaken_successor());
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
	delete self;
}

void
jive_cfg_validate(const struct jive_cfg * self)
{
	jive_cfg_node * node;
	JIVE_LIST_ITERATE(self->nodes, node, cfg_node_list) {
		if (dynamic_cast<jive_cfg_enter_node*>(node)) {
			JIVE_ASSERT(node->predecessors().size() == 0);
			JIVE_ASSERT(node->taken_successor() == nullptr);
			JIVE_ASSERT(node->nottaken_successor() != nullptr);
		}

		if (dynamic_cast<jive_cfg_exit_node*>(node)) {
			JIVE_ASSERT(node->taken_successor() == NULL);
			JIVE_ASSERT(node->nottaken_successor() == NULL);
		}

		if (dynamic_cast<jive_cfg_exit_node*>(node))
			continue;

		JIVE_ASSERT(node->nottaken_successor() != NULL);

		/* check whether successors are in the predecessor list */
		size_t n;
		jive_cfg_node * successor = node->nottaken_successor();
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
