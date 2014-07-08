/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/common.h>
#include <jive/frontend/basic_block.h>
#include <jive/frontend/cfg.h>
#include <jive/frontend/tac/bitstring/arithmetic/bitsum.h>
#include <jive/frontend/clg.h>
#include <jive/frontend/clg_node.h>

#include <string.h>
#include <stdio.h>

namespace jive {
namespace frontend {

bitsum_code::~bitsum_code() noexcept {}

bitsum_code::bitsum_code(jive_basic_block * basic_block,
	three_address_code * summand1, three_address_code * summand2)
	: three_address_code(basic_block, {summand1, summand2})
{}

std::string
bitsum_code::debug_string() const
{
	char tmp[64];
	snprintf(tmp, sizeof(tmp), "%p + %p", operands[0], operands[1]);
	return std::string(tmp);
}

}
}

jive::frontend::three_address_code *
jive_bitsum_code_create(jive_basic_block * basic_block,
	jive::frontend::three_address_code * summand1, jive::frontend::three_address_code * summand2)
{
	return new jive::frontend::bitsum_code(basic_block, summand1, summand2);
}
