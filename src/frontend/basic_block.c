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

jive_basic_block::~jive_basic_block()
{
	while (three_address_codes.first)
		delete three_address_codes.first;
}

std::string
jive_basic_block::debug_string() const
{
	std::string label;

	char tmp[32];
	snprintf(tmp, sizeof(tmp), "%p\\n", this);
	label.append(tmp);

	jive_three_address_code * tac;
	JIVE_LIST_ITERATE(three_address_codes, tac, basic_block_three_address_codes_list) {
		snprintf(tmp, sizeof(tmp), "%p : ", tac);
		label.append(tmp);
		label.append(tac->debug_string());
		label.append("\\n");
	}

	return label;
}

void
jive_basic_block_get_label_(const struct jive_cfg_node * self, struct jive_buffer * buffer);

const jive_cfg_node_class JIVE_BASIC_BLOCK = {
	parent : &JIVE_CFG_NODE,
	name : "BASIC BLOCK",
	fini : nullptr, /* override */
	get_label : jive_basic_block_get_label_, /* override */
};

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
	jive_cfg_node_init_(self, cfg);

	self->three_address_codes.first = 0;
	self->three_address_codes.last = 0;
}

struct jive_cfg_node *
jive_basic_block_create(struct jive_cfg * cfg)
{
	jive_basic_block * node = new jive_basic_block;
	node->class_ = &JIVE_BASIC_BLOCK;
	jive_basic_block_init_(node, cfg);
	return node;
}
