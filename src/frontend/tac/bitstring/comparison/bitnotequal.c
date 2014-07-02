/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/common.h>
#include <jive/frontend/basic_block.h>
#include <jive/frontend/cfg.h>
#include <jive/frontend/tac/bitstring/comparison/bitnotequal.h>
#include <jive/frontend/clg_node.h>
#include <jive/frontend/clg.h>
#include <jive/util/buffer.h>

#include <string.h>
#include <stdio.h>

jive_bitnotequal_code::~jive_bitnotequal_code() noexcept {}

jive_bitnotequal_code::jive_bitnotequal_code(struct jive_basic_block * basic_block,
	jive_three_address_code * op1, jive_three_address_code * op2)
	: jive_three_address_code(basic_block, {op1, op2})
{}

std::string
jive_bitnotequal_code::debug_string() const
{
	char tmp[64];
	snprintf(tmp, sizeof(tmp), "%p != %p", operands[0], operands[1]);
	return std::string(tmp);
}

jive_three_address_code *
jive_bitnotequal_code_create(struct jive_basic_block * basic_block, jive_three_address_code * op1,
	jive_three_address_code * op2)
{
	return new jive_bitnotequal_code(basic_block, op1, op2);
}
