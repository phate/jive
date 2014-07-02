/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_TAC_THREE_ADDRESS_CODE_H
#define JIVE_FRONTEND_TAC_THREE_ADDRESS_CODE_H

#include <stdbool.h>
#include <stddef.h>

#include <string>
#include <vector>

class jive_three_address_code {
public:
	virtual ~jive_three_address_code() noexcept;

	virtual std::string debug_string() const = 0;

	struct jive_basic_block * basic_block;

	std::vector<jive_three_address_code*> operands;

	struct {
		jive_three_address_code * prev;
		jive_three_address_code * next;
	} basic_block_three_address_codes_list;
};

void
jive_three_address_code_destroy(jive_three_address_code * self);

#endif
