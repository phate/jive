/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_TAC_BITSTRING_BITCONSTANT_H
#define JIVE_FRONTEND_TAC_BITSTRING_BITCONSTANT_H

#include <jive/frontend/tac/three_address_code.h>

#include <vector>

class jive_bitconstant_code final : public jive_three_address_code {
public:
	virtual ~jive_bitconstant_code() noexcept;

	jive_bitconstant_code(struct jive_basic_block * basic_block, size_t nbits, const char * bits);

	virtual std::string debug_string() const override;

	std::vector<char> bits; /* [LSB, ..., MSB] */
};

struct jive_three_address_code *
jive_bitconstant_code_create(struct jive_basic_block * basic_block, size_t nbits, const char * bits);

#endif
