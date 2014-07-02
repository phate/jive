/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_TAC_BITSTRING_BITCONSTANT_H
#define JIVE_FRONTEND_TAC_BITSTRING_BITCONSTANT_H

#include <jive/frontend/tac/three_address_code.h>

#include <vector>

extern const jive_three_address_code_class JIVE_BITCONSTANT_CODE;

typedef struct jive_bitconstant_code_attrs jive_bitconstant_code_attrs;

struct jive_bitconstant_code_attrs {
	jive_three_address_code_attrs base;
	std::vector<char> bits; /* [LSB, ..., MSB] */
};

class jive_bitconstant_code final : public jive_three_address_code {
public:
	virtual ~jive_bitconstant_code() noexcept;

	jive_bitconstant_code_attrs attrs;
};

struct jive_three_address_code *
jive_bitconstant_code_create(struct jive_basic_block * basic_block, size_t nbits, const char * bits);

#endif
