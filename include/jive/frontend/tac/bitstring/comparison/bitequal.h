/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_TAC_BITSTRING_COMPARISON_BITEQUAL_H
#define JIVE_FRONTEND_TAC_BITSTRING_COMPARISON_BITEQUAL_H

#include <jive/frontend/tac/three_address_code.h>

extern const jive_three_address_code_class JIVE_BITEQUAL_CODE;

typedef struct jive_bitequal_code jive_bitequal_code;

struct jive_bitequal_code {
	jive_three_address_code base;
};

static inline jive_bitequal_code *
jive_bitequal_code_cast(const struct jive_three_address_code * tac)
{
	if (jive_three_address_code_isinstance(tac, &JIVE_BITEQUAL_CODE))
		return (jive_bitequal_code *) tac;
	else
		return 0;
}

static inline const jive_bitequal_code *
jive_bitequal_code_const_cast(const struct jive_three_address_code * tac)
{
	if (jive_three_address_code_isinstance(tac, &JIVE_BITEQUAL_CODE))
		return (const jive_bitequal_code *) tac;
	else
		return 0;
}

struct jive_three_address_code *
jive_bitequal_code_create(struct jive_basic_block * basic_block,
	struct jive_three_address_code * op1, struct jive_three_address_code * op2);

#endif
