/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_TAC_BITSTRING_COMPARISON_BITEQUAL_H
#define JIVE_FRONTEND_TAC_BITSTRING_COMPARISON_BITEQUAL_H

#include <jive/frontend/tac/three_address_code.h>

class jive_basic_block;

namespace jive {
namespace frontend {

class bitequal_code final : public three_address_code {
public:
	virtual ~bitequal_code() noexcept;

	bitequal_code(jive_basic_block * basic_block, three_address_code * op1, three_address_code * op2);

	virtual std::string debug_string() const override;
};

}
}

jive::frontend::three_address_code *
jive_bitequal_code_create(jive_basic_block * basic_block,
	jive::frontend::three_address_code * op1, jive::frontend::three_address_code * op2);

#endif
