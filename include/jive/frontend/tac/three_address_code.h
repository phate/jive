/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_TAC_THREE_ADDRESS_CODE_H
#define JIVE_FRONTEND_TAC_THREE_ADDRESS_CODE_H

#include <string>
#include <vector>

class jive_basic_block;

namespace jive {
namespace frontend {

class three_address_code {
public:
	virtual ~three_address_code() noexcept;

protected:
	three_address_code(jive_basic_block * basic_block,
		std::initializer_list<three_address_code*> operands);

	three_address_code(jive_basic_block * basic_block, std::vector<three_address_code*> & operands);

public:
	virtual std::string debug_string() const = 0;

	jive_basic_block * basic_block;

	std::vector<three_address_code*> operands;

	struct {
		three_address_code * prev;
		three_address_code * next;
	} basic_block_three_address_codes_list;
};

}
}

#endif
