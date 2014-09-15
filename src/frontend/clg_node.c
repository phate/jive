/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/common.h>
#include <jive/context.h>
#include <jive/frontend/cfg.h>
#include <jive/frontend/clg.h>
#include <jive/frontend/clg_node.h>
#include <jive/util/buffer.h>
#include <jive/util/list.h>

static void
jive_clg_node_fini_(struct jive_clg_node * self);

static void
jive_clg_node_get_label_(const struct jive_clg_node * self, struct jive_buffer * buffer);

static jive_clg_node *
jive_clg_node_create_(struct jive_clg * clg, const char * name);

const struct jive_clg_node_class JIVE_CLG_NODE = {
	parent : 0,
	name : "CLG_NODE",
	fini : jive_clg_node_fini_,
	get_label : jive_clg_node_get_label_,
	create : jive_clg_node_create_
};

void
jive_clg_node_init_(struct jive_clg_node * self, struct jive_clg * clg, const char * name)
{
	self->clg = clg;

	self->ncalls = 0;
	self->calls = 0;

	JIVE_LIST_PUSH_BACK(clg->nodes, self, clg_node_list);

	self->name = jive_context_strdup(clg->context, name);
	self->cfg = NULL;
}

void
jive_clg_node_fini_(struct jive_clg_node * self)
{
	JIVE_LIST_REMOVE(self->clg->nodes, self, clg_node_list);

	jive_context_free(self->clg->context, self->calls);
	jive_context_free(self->clg->context, self->name);

	delete self->cfg;
}

void
jive_clg_node_get_label_(const jive_clg_node * self, struct jive_buffer * buffer)
{
	jive_buffer_putstr(buffer, self->name);
}

struct jive_clg_node *
jive_clg_node_create_(struct jive_clg * clg, const char * name)
{
	jive_clg_node * node = jive_context_malloc(clg->context, sizeof(*node));

	node->class_ = &JIVE_CLG_NODE;
	jive_clg_node_init_(node, clg, name);

	return node;
}

struct jive_clg_node *
jive_clg_node_create(struct jive_clg * clg, const char * name)
{
	return jive_clg_node_create_(clg, name);
}

void
jive_clg_node_add_call(struct jive_clg_node * self, struct jive_clg_node * callee)
{
	size_t n;
	for (n = 0; n < self->ncalls; n++) {
		if (self->calls[n] == callee)
			return;
	}

	self->ncalls++;
	self->calls = jive_context_realloc(self->clg->context, self->calls,
		sizeof(jive_clg_node *) * self->ncalls);
	self->calls[self->ncalls-1] = callee;
}

void
jive_clg_node_destroy(struct jive_clg_node * self)
{
	self->class_->fini(self);
	jive_context_free(self->clg->context, self);
}
