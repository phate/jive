/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_TAC_BITSTRING_BITCONSTANT_H
#define JIVE_FRONTEND_TAC_BITSTRING_BITCONSTANT_H

#include <jive/frontend/tac/three_address_code.h>

#include <vector>

extern const jive_three_address_code_class JIVE_BITCONSTANT_CODE;

typedef struct jive_bitconstant_code jive_bitconstant_code;
typedef struct jive_bitconstant_code_attrs jive_bitconstant_code_attrs;

struct jive_bitconstant_code_attrs {
	jive_three_address_code_attrs base;
	std::vector<char> bits; /* [LSB, ..., MSB] */
};

struct jive_bitconstant_code {
	jive_three_address_code base;
	jive_bitconstant_code_attrs attrs;
};

static inline struct jive_bitconstant_code *
jive_bitconstant_code_cast(const struct jive_three_address_code * tac)
{
	if (jive_three_address_code_isinstance(tac, &JIVE_BITCONSTANT_CODE))
		return (jive_bitconstant_code *) tac;
	else
		return 0;
}

static inline const struct jive_bitconstant_code *
jive_bitconstant_code_const_cast(const struct jive_three_address_code * tac)
{
	if (jive_three_address_code_isinstance(tac, &JIVE_BITCONSTANT_CODE))
		return (const jive_bitconstant_code *) tac;
	else
		return 0;
}

struct jive_three_address_code *
jive_bitconstant_code_create(struct jive_basic_block * basic_block, size_t nbits, const char * bits);

#endif
