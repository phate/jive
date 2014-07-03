/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_CFG_H
#define JIVE_FRONTEND_CFG_H

#include <jive/frontend/cfg_node.h>

#include <stdbool.h>
#include <stddef.h>

/* cfg enter node */

class jive_cfg_enter_node final : public jive_cfg_node {
public:
	virtual ~jive_cfg_enter_node() noexcept;

	jive_cfg_enter_node(struct jive_cfg * cfg) noexcept;

	virtual std::string debug_string() const override;
};

/* cfg exit node */

class jive_cfg_exit_node final : public jive_cfg_node {
public:
	virtual ~jive_cfg_exit_node() noexcept;

	jive_cfg_exit_node(struct jive_cfg * cfg) noexcept;

	virtual std::string debug_string() const override;
};

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
