/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/common.h>
#include <jive/frontend/basic_block.h>
#include <jive/frontend/cfg.h>
#include <jive/frontend/tac/bitstring/comparison/bitequal.h>
#include <jive/frontend/clg_node.h>
#include <jive/frontend/clg.h>

#include <string.h>
#include <stdio.h>

namespace jive {
namespace frontend {

bitequal_code::~bitequal_code() noexcept {}

bitequal_code::bitequal_code(jive_basic_block * basic_block,
	three_address_code * op1, three_address_code * op2)
	: three_address_code(basic_block, {op1, op2})
{}

std::string
bitequal_code::debug_string() const
{
	char tmp[64];
	snprintf(tmp, sizeof(tmp), "%p == %p", operands[0], operands[1]);
	return std::string(tmp);
}

}
}

jive::frontend::three_address_code *
jive_bitequal_code_create(jive_basic_block * basic_block, jive::frontend::three_address_code * op1,
	jive::frontend::three_address_code * op2)
{
	return new jive::frontend::bitequal_code(basic_block, op1, op2);
}
