/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_TAC_BITSTRING_COMPARISON_BITNOTEQUAL_H
#define JIVE_FRONTEND_TAC_BITSTRING_COMPARISON_BITNOTEQUAL_H

#include <jive/frontend/tac/three_address_code.h>

extern const jive_three_address_code_class JIVE_BITNOTEQUAL_CODE;

class jive_bitnotequal_code final : public jive_three_address_code {
};

static inline jive_bitnotequal_code *
jive_bitnotequal_code_cast(struct jive_three_address_code * tac)
{
	if (jive_three_address_code_isinstance(tac, &JIVE_BITNOTEQUAL_CODE))
		return (jive_bitnotequal_code *) tac;
	else
		return 0;
}

struct jive_three_address_code *
jive_bitnotequal_code_create(struct jive_basic_block * basic_block,
	struct jive_three_address_code * op1, struct jive_three_address_code * op2);

#endif
