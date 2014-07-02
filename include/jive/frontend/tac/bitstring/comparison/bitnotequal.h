/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_TAC_BITSTRING_COMPARISON_BITNOTEQUAL_H
#define JIVE_FRONTEND_TAC_BITSTRING_COMPARISON_BITNOTEQUAL_H

#include <jive/frontend/tac/three_address_code.h>

class jive_bitnotequal_code final : public jive_three_address_code {
public:
	virtual ~jive_bitnotequal_code() noexcept;

	virtual std::string debug_string() const override;
};

struct jive_three_address_code *
jive_bitnotequal_code_create(struct jive_basic_block * basic_block,
	struct jive_three_address_code * op1, struct jive_three_address_code * op2);

#endif
