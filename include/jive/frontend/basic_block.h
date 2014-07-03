/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_BASIC_BLOCK_H
#define JIVE_FRONTEND_BASIC_BLOCK_H

#include <jive/frontend/cfg_node.h>

extern const jive_cfg_node_class JIVE_BASIC_BLOCK;

class jive_basic_block final : public jive_cfg_node {
public:

	struct {
		struct jive_three_address_code * first;
		struct jive_three_address_code * last;
	} three_address_codes;
};

static inline struct jive_basic_block *
jive_basic_block_cast(const struct jive_cfg_node * node)
{
	if (jive_cfg_node_isinstance(node, &JIVE_BASIC_BLOCK))
		return (struct jive_basic_block *) node;
	else
		return 0;
}

struct jive_cfg_node *
jive_basic_block_create(struct jive_cfg * cfg);

#endif
