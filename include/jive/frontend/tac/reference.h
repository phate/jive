/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_TAC_REFERENCE_H
#define JIVE_FRONTEND_TAC_REFERENCE_H

#include <jive/frontend/tac/three_address_code.h>

class jive_variable_code;

class jive_reference_code final : public jive_three_address_code {
public:
	virtual ~jive_reference_code() noexcept;

	jive_reference_code(struct jive_basic_block * basic_block, jive_variable_code * variable);

	virtual std::string debug_string() const override;
};

struct jive_three_address_code *
jive_reference_code_create(struct jive_basic_block * basic_block, jive_variable_code * variable);

static inline struct jive_three_address_code *
jive_reference_code_get_reference(const struct jive_reference_code * self)
{
	return self->operands[0];
}

static inline void
jive_reference_code_set_reference(struct jive_reference_code * self,
	struct jive_three_address_code * tac)
{
	self->operands[0] = tac;
}

#endif
