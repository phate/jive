/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_TAC_BITSTRING_ARITHMETIC_BITSUM_H
#define JIVE_FRONTEND_TAC_BITSTRING_ARITHMETIC_BITSUM_H

#include <jive/frontend/tac/three_address_code.h>

class jive_basic_block;

namespace jive {
namespace frontend {

class bitsum_code final : public three_address_code {
public:
	virtual ~bitsum_code() noexcept;

	bitsum_code(jive_basic_block * basic_block, jive::frontend::three_address_code * summand1,
		jive::frontend::three_address_code * summand2);

	virtual std::string debug_string() const override;
};

}
}

jive::frontend::three_address_code *
jive_bitsum_code_create(jive_basic_block * basic_block,
	jive::frontend::three_address_code * summand1, jive::frontend::three_address_code * summand2);

#endif
