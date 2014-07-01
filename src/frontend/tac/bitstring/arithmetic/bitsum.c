/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/common.h>
#include <jive/frontend/basic_block.h>
#include <jive/frontend/cfg.h>
#include <jive/frontend/tac/bitstring/arithmetic/bitsum.h>
#include <jive/frontend/tac/three_address_code-private.h>
#include <jive/frontend/clg.h>
#include <jive/frontend/clg_node.h>
#include <jive/util/buffer.h>

#include <string.h>
#include <stdio.h>

static void
jive_bitsum_code_get_label_(const struct jive_three_address_code * self,
	struct jive_buffer * buffer);

static struct jive_three_address_code *
jive_bitsum_code_create_(struct jive_basic_block * basic_block,
	const struct jive_three_address_code_attrs * attrs,
	size_t noperands, struct jive_three_address_code * const operands[]);

const struct jive_three_address_code_class JIVE_BITSUM_CODE = {
	parent : &JIVE_THREE_ADDRESS_CODE,
	name : "BITSUM",
	fini : jive_three_address_code_fini_, /* inherit */
	get_label : jive_bitsum_code_get_label_, /* override */
	get_attrs : jive_three_address_code_get_attrs_, /* inherit */
	create : jive_bitsum_code_create_ /* override */
};

static void
jive_bitsum_code_init_(jive_bitsum_code * self, struct jive_basic_block * basic_block,
	struct jive_three_address_code * summand1, struct jive_three_address_code * summand2)
{
	jive_three_address_code * tmparray0[] = {summand1, summand2};
	jive_three_address_code_init_(self, basic_block, 2, tmparray0);
}

static void
jive_bitsum_code_get_label_(const struct jive_three_address_code * self, struct jive_buffer * buffer)
{
	char tmp[64];
	snprintf(tmp, sizeof(tmp), "%p + %p", self->operands[0], self->operands[1]);
	jive_buffer_putstr(buffer, tmp);
}

static struct jive_three_address_code *
jive_bitsum_code_create_(struct jive_basic_block * basic_block,
	const struct jive_three_address_code_attrs * attrs,
	size_t noperands, struct jive_three_address_code * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 2);

	jive_bitsum_code * sum = new jive_bitsum_code;
	sum->class_ = &JIVE_BITSUM_CODE;
	jive_bitsum_code_init_(sum, basic_block, operands[0], operands[1]);
	return sum;
}

jive_three_address_code *
jive_bitsum_code_create(struct jive_basic_block * basic_block, jive_three_address_code * summand1,
	jive_three_address_code * summand2)
{
	jive_three_address_code * tmparray1[] = {summand1, summand2};
	return jive_bitsum_code_create_(basic_block, NULL,
		2, tmparray1);
}
