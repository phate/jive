/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/frontend/clg.h>
#include <jive/frontend/basic_block.h>
#include <jive/frontend/cfg.h>

#include <stdio.h>

jive_basic_block::~jive_basic_block()
{
}

jive_basic_block::jive_basic_block(struct jive_cfg * cfg) noexcept
	: jive_cfg_node(cfg)
{
}

std::string
jive_basic_block::debug_string() const
{
	std::string label;

	char tmp[32];
	snprintf(tmp, sizeof(tmp), "%p\\n", this);
	label.append(tmp);

	return label;
}

struct jive_cfg_node *
jive_basic_block_create(struct jive_cfg * cfg)
{
	return new jive_basic_block(cfg);
}
