/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/frontend/basic_block.h>
#include <jive/frontend/cfg.h>

#include <stdio.h>

namespace jive {
namespace frontend {

basic_block::~basic_block() {}

basic_block::basic_block(jive::frontend::cfg & cfg) noexcept
	: cfg_node(cfg)
{}

std::string
basic_block::debug_string() const
{
	std::string label;

	char tmp[32];
	snprintf(tmp, sizeof(tmp), "%p\\n", this);
	label.append(tmp);

	return label;
}

}
}
