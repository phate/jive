/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/frontend/basic_block.h>
#include <jive/frontend/tac/three_address_code.h>
#include <jive/util/list.h>

jive_three_address_code::~jive_three_address_code() noexcept
{
	JIVE_LIST_REMOVE(basic_block->three_address_codes, this, basic_block_three_address_codes_list);
}

jive_three_address_code::jive_three_address_code(struct jive_basic_block * basic_block_,
	std::initializer_list<jive_three_address_code*> operands_)
	: basic_block(basic_block_)
	, operands(operands_.begin(), operands_.end())
{
	basic_block_three_address_codes_list.prev = NULL;
	basic_block_three_address_codes_list.next = NULL;
	JIVE_LIST_PUSH_BACK(basic_block->three_address_codes, this, basic_block_three_address_codes_list);
}

jive_three_address_code::jive_three_address_code(struct jive_basic_block * basic_block_,
	std::vector<jive_three_address_code*> & operands_)
	: basic_block(basic_block_)
	, operands(operands_)
{
	basic_block_three_address_codes_list.prev = NULL;
	basic_block_three_address_codes_list.next = NULL;
	JIVE_LIST_PUSH_BACK(basic_block->three_address_codes, this, basic_block_three_address_codes_list);
}
