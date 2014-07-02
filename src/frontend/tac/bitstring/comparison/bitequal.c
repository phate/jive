/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/common.h>
#include <jive/frontend/basic_block.h>
#include <jive/frontend/cfg.h>
#include <jive/frontend/tac/bitstring/comparison/bitequal.h>
#include <jive/frontend/tac/three_address_code-private.h>
#include <jive/frontend/clg_node.h>
#include <jive/frontend/clg.h>
#include <jive/util/buffer.h>

#include <string.h>
#include <stdio.h>

jive_bitequal_code::~jive_bitequal_code() noexcept {}

static void
jive_bitequal_code_get_label_(const struct jive_three_address_code * self,
	struct jive_buffer * buffer);

const struct jive_three_address_code_class JIVE_BITEQUAL_CODE = {
	parent : &JIVE_THREE_ADDRESS_CODE,
	name : "BITEQUAL",
	fini : nullptr, /* inherit */
	get_label : jive_bitequal_code_get_label_, /* override */
	get_attrs : jive_three_address_code_get_attrs_, /* inherit */
};

static void
jive_bitequal_code_init_(jive_bitequal_code * self, struct jive_basic_block * basic_block,
	struct jive_three_address_code * op1, struct jive_three_address_code * op2)
{
	jive_three_address_code * tmparray0[] = {op1, op2};
	jive_three_address_code_init_(self, basic_block, 2, tmparray0);
}

static void
jive_bitequal_code_get_label_(const struct jive_three_address_code * self,
	struct jive_buffer * buffer)
{
	char tmp[64];
	snprintf(tmp, sizeof(tmp), "%p == %p", self->operands[0], self->operands[1]);
	jive_buffer_putstr(buffer, tmp);
}

jive_three_address_code *
jive_bitequal_code_create(struct jive_basic_block * basic_block, jive_three_address_code * op1,
	jive_three_address_code * op2)
{
	jive_bitequal_code * equal = new jive_bitequal_code;
	equal->class_ = &JIVE_BITEQUAL_CODE;
	jive_bitequal_code_init_(equal, basic_block, op1, op2);
	return equal;
}
