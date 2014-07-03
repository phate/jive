/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_CFG_H
#define JIVE_FRONTEND_CFG_H

#include <jive/frontend/cfg_node.h>

#include <stdbool.h>
#include <stddef.h>

struct jive_buffer;

/* cfg enter node */
extern const jive_cfg_node_class JIVE_CFG_ENTER_NODE;

class jive_cfg_enter_node final : public jive_cfg_node {
};

JIVE_EXPORTED_INLINE const jive_cfg_enter_node *
jive_cfg_enter_node_const_cast(const jive_cfg_node * self)
{
	if (jive_cfg_node_isinstance(self, &JIVE_CFG_ENTER_NODE))
		return (const jive_cfg_enter_node *)self;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_cfg_enter_node *
jive_cfg_enter_node_cast(jive_cfg_node * self)
{
	if (jive_cfg_node_isinstance(self, &JIVE_CFG_ENTER_NODE))
		return (jive_cfg_enter_node *)self;
	else
		return NULL;
}

/* cfg exit node */

extern const jive_cfg_node_class JIVE_CFG_EXIT_NODE;

class jive_cfg_exit_node final : public jive_cfg_node {
};

JIVE_EXPORTED_INLINE const jive_cfg_exit_node *
jive_cfg_exit_node_const_cast(const jive_cfg_node * self)
{
	if (jive_cfg_node_isinstance(self, &JIVE_CFG_EXIT_NODE))
		return (const jive_cfg_exit_node *)self;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_cfg_exit_node *
jive_cfg_exit_node_cast(jive_cfg_node * self)
{
	if (jive_cfg_node_isinstance(self, &JIVE_CFG_EXIT_NODE))
		return (jive_cfg_exit_node *)self;
	else
		return NULL;
}

/* cfg */

typedef struct jive_cfg jive_cfg;

struct jive_cfg {
	struct jive_clg_node * clg_node;
	struct jive_context * context;

	struct jive_cfg_node * enter;
	struct jive_cfg_node * exit;

	struct {
		struct jive_cfg_node * first;
		struct jive_cfg_node * last;
	} nodes;
};

jive_cfg *
jive_cfg_create(struct jive_clg_node * clg_node);

bool
jive_cfg_is_empty(const struct jive_cfg * self);

void
jive_cfg_convert_dot(const struct jive_cfg * self, struct jive_buffer * buffer);

void
jive_cfg_view(const struct jive_cfg * self);

void
jive_cfg_destroy(struct jive_cfg * self);

void
jive_cfg_validate(const struct jive_cfg * self);

#endif
