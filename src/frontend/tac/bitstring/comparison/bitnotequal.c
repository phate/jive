/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/common.h>
#include <jive/context.h>
#include <jive/frontend/basic_block.h>
#include <jive/frontend/cfg.h>
#include <jive/frontend/tac/bitstring/comparison/bitnotequal.h>
#include <jive/frontend/tac/three_address_code-private.h>
#include <jive/frontend/clg_node.h>
#include <jive/frontend/clg.h>
#include <jive/util/buffer.h>

#include <string.h>
#include <stdio.h>

static void
jive_bitnotequal_code_get_label_(const struct jive_three_address_code * self,
	struct jive_buffer * buffer);

static struct jive_three_address_code *
jive_bitnotequal_code_create_(struct jive_basic_block * basic_block,
	const struct jive_three_address_code_attrs * attrs,
	size_t noperands, struct jive_three_address_code * const operands[]);

const struct jive_three_address_code_class JIVE_BITNOTEQUAL_CODE = {
	.parent = &JIVE_THREE_ADDRESS_CODE,
	.name = "BITNOTEQUAL",
	.fini = jive_three_address_code_fini_, /* inherit */
	.get_label = jive_bitnotequal_code_get_label_, /* override */
	.get_attrs = jive_three_address_code_get_attrs_, /* inherit */
	.create = jive_bitnotequal_code_create_ /* override */
};

static void
jive_bitnotequal_code_init_(jive_bitnotequal_code * self, struct jive_basic_block * basic_block,
	struct jive_three_address_code * op1, struct jive_three_address_code * op2)
{
	jive_three_address_code_init_(self, basic_block,
	2, (jive_three_address_code *[]){op1, op2});
}

static void
jive_bitnotequal_code_get_label_(const struct jive_three_address_code * self,
	struct jive_buffer * buffer)
{
	char tmp[64];
	snprintf(tmp, sizeof(tmp), "%p != %p", self->operands[0], self->operands[1]);
	jive_buffer_putstr(buffer, tmp);
}

static struct jive_three_address_code *
jive_bitnotequal_code_create_(struct jive_basic_block * basic_block,
	const struct jive_three_address_code_attrs * attrs,
	size_t noperands, struct jive_three_address_code * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 2);

	jive_context * context = basic_block->base.cfg->clg_node->clg->context;	
	jive_bitnotequal_code * notequal = jive_context_malloc(context, sizeof(*notequal));
	notequal->class_ = &JIVE_BITNOTEQUAL_CODE;
	jive_bitnotequal_code_init_(notequal, basic_block, operands[0], operands[1]);
	return notequal;
}

jive_three_address_code *
jive_bitnotequal_code_create(struct jive_basic_block * basic_block, jive_three_address_code * op1,
	jive_three_address_code * op2)
{
	return jive_bitnotequal_code_create_(basic_block, NULL,
		2, (jive_three_address_code *[]){op1, op2});
}

