/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/common.h>
#include <jive/frontend/basic_block.h>
#include <jive/frontend/cfg.h>
#include <jive/frontend/tac/bitstring/comparison/bitnotequal.h>
#include <jive/frontend/tac/three_address_code-private.h>
#include <jive/frontend/clg_node.h>
#include <jive/frontend/clg.h>
#include <jive/util/buffer.h>

#include <string.h>
#include <stdio.h>

jive_bitnotequal_code::~jive_bitnotequal_code() noexcept {}

std::string
jive_bitnotequal_code::debug_string() const
{
	char tmp[64];
	snprintf(tmp, sizeof(tmp), "%p != %p", operands[0], operands[1]);
	return std::string(tmp);
}

const struct jive_three_address_code_class JIVE_BITNOTEQUAL_CODE = {
	parent : &JIVE_THREE_ADDRESS_CODE,
	name : "BITNOTEQUAL",
	fini : nullptr, /* inherit */
	get_attrs : jive_three_address_code_get_attrs_, /* inherit */
};

static void
jive_bitnotequal_code_init_(jive_bitnotequal_code * self, struct jive_basic_block * basic_block,
	struct jive_three_address_code * op1, struct jive_three_address_code * op2)
{
	jive_three_address_code * tmparray0[] = {op1, op2};
	jive_three_address_code_init_(self, basic_block,
	2, tmparray0);
}

jive_three_address_code *
jive_bitnotequal_code_create(struct jive_basic_block * basic_block, jive_three_address_code * op1,
	jive_three_address_code * op2)
{
	jive_bitnotequal_code * notequal = new jive_bitnotequal_code;
	notequal->class_ = &JIVE_BITNOTEQUAL_CODE;
	jive_bitnotequal_code_init_(notequal, basic_block, op1, op2);
	return notequal;
}
