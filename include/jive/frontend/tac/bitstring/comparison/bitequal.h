/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_TAC_BITSTRING_COMPARISON_BITEQUAL_H
#define JIVE_FRONTEND_TAC_BITSTRING_COMPARISON_BITEQUAL_H

#include <jive/frontend/tac/three_address_code.h>

extern const jive_three_address_code_class JIVE_BITEQUAL_CODE;

class jive_bitequal_code final : public jive_three_address_code {
public:
	virtual ~jive_bitequal_code() noexcept;
};

struct jive_three_address_code *
jive_bitequal_code_create(struct jive_basic_block * basic_block,
	struct jive_three_address_code * op1, struct jive_three_address_code * op2);

#endif
