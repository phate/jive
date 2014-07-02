/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/frontend/clg.h>
#include <jive/frontend/clg_node.h>
#include <jive/frontend/basic_block.h>
#include <jive/frontend/cfg.h>
#include <jive/frontend/cfg_node-private.h>
#include <jive/frontend/tac/three_address_code.h>
#include <jive/util/buffer.h>
#include <jive/util/list.h>

#include <stdio.h>

void
jive_basic_block_fini_(struct jive_cfg_node * self);

void
jive_basic_block_get_label_(const struct jive_cfg_node * self, struct jive_buffer * buffer);

struct jive_cfg_node *
jive_basic_block_create_(struct jive_cfg * cfg);

const jive_cfg_node_class JIVE_BASIC_BLOCK = {
	parent : &JIVE_CFG_NODE,
	name : "BASIC BLOCK",
	fini : jive_basic_block_fini_, /* override */
	get_label : jive_basic_block_get_label_, /* override */
	create : jive_basic_block_create_ /* override */
};

void
jive_basic_block_fini_(struct jive_cfg_node * self_)
{
	struct jive_basic_block * self = (struct jive_basic_block *)self_;

	while (self->three_address_codes.first)
		jive_three_address_code_destroy(self->three_address_codes.first);

	jive_cfg_node_fini_(self_);
}

void
jive_basic_block_get_label_(const struct jive_cfg_node * self_, struct jive_buffer * buffer)
{
	struct jive_basic_block * self = (struct jive_basic_block *)self_;

	char tmp[32];
	snprintf(tmp, sizeof(tmp), "%p\\n", self_);
	jive_buffer_putstr(buffer, tmp);

	jive_three_address_code * tac;
	JIVE_LIST_ITERATE(self->three_address_codes, tac, basic_block_three_address_codes_list) {
		snprintf(tmp, sizeof(tmp), "%p : ", tac);
		jive_buffer_putstr(buffer, tmp);
		jive_buffer_putstr(buffer, tac->debug_string().c_str());
		jive_buffer_putstr(buffer, "\\n");
	}
}

void
jive_basic_block_init_(struct jive_basic_block * self, struct jive_cfg * cfg)
{
	jive_cfg_node_init_(&self->base, cfg);

	self->three_address_codes.first = 0;
	self->three_address_codes.last = 0;
}

struct jive_cfg_node *
jive_basic_block_create_(struct jive_cfg * cfg)
{
	struct jive_basic_block * node = new jive_basic_block;
	node->base.class_ = &JIVE_BASIC_BLOCK;
	jive_basic_block_init_(node, cfg);
	return &node->base;
}

void
jive_basic_block_transfer_tacs(struct jive_basic_block * self, struct jive_basic_block * other)
{
	jive_three_address_code * tac, * next;
	JIVE_LIST_ITERATE_SAFE(self->three_address_codes, tac, next,
		basic_block_three_address_codes_list) {
		JIVE_LIST_REMOVE(self->three_address_codes, tac, basic_block_three_address_codes_list);
		tac->basic_block = other;
		JIVE_LIST_PUSH_BACK(other->three_address_codes, tac, basic_block_three_address_codes_list);
	}
}

struct jive_cfg_node *
jive_basic_block_create(struct jive_cfg * cfg)
{
	return jive_basic_block_create_(cfg);
}
