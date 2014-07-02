/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/frontend/basic_block.h>
#include <jive/frontend/cfg.h>
#include <jive/frontend/tac/three_address_code-private.h>
#include <jive/frontend/tac/three_address_code.h>
#include <jive/util/buffer.h>
#include <jive/util/list.h>

#include <string.h>

jive_three_address_code::~jive_three_address_code() noexcept
{
	JIVE_LIST_REMOVE(basic_block->three_address_codes, this, basic_block_three_address_codes_list);
}

const struct jive_three_address_code_class JIVE_THREE_ADDRESS_CODE = {
	parent : 0,
	name : "THREE_ADDRESS_CODE",
	fini : nullptr,
	get_attrs : jive_three_address_code_get_attrs_,
};

void
jive_three_address_code_init_(jive_three_address_code * self,
	struct jive_basic_block * basic_block,
	size_t noperands, jive_three_address_code * const operands[])
{
	self->basic_block = basic_block;
	self->basic_block_three_address_codes_list.prev = NULL;
	self->basic_block_three_address_codes_list.next = NULL;

	for (size_t i = 0; i < noperands; i++)
		self->operands.push_back(operands[i]);

	JIVE_LIST_PUSH_BACK(basic_block->three_address_codes, self, basic_block_three_address_codes_list);
}

const jive_three_address_code_attrs *
jive_three_address_code_get_attrs_(const jive_three_address_code * self)
{
	return 0;
}

void
jive_three_address_code_destroy(jive_three_address_code * self)
{
	delete self;
}
