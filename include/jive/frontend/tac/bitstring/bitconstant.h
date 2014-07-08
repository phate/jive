/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_TAC_BITSTRING_BITCONSTANT_H
#define JIVE_FRONTEND_TAC_BITSTRING_BITCONSTANT_H

#include <jive/frontend/tac/three_address_code.h>

#include <vector>

class jive_basic_block;

namespace jive {
namespace frontend {

class bitconstant_code final : public three_address_code {
public:
	virtual ~bitconstant_code() noexcept;

	bitconstant_code(jive_basic_block * basic_block, size_t nbits, const char * bits);

	virtual std::string debug_string() const override;

	std::vector<char> bits; /* [LSB, ..., MSB] */
};

}
}

jive::frontend::three_address_code *
jive_bitconstant_code_create(jive_basic_block * basic_block, size_t nbits,
	const char * bits);

#endif
