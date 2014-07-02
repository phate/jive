/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_TAC_ASSIGNMENT_H
#define JIVE_FRONTEND_TAC_ASSIGNMENT_H

#include <jive/frontend/tac/three_address_code.h>
#include <jive/frontend/tac/variable.h>

extern const jive_three_address_code_class JIVE_ASSIGNMENT_CODE;

class jive_assignment_code final : public jive_three_address_code {
public:
	virtual ~jive_assignment_code() noexcept;
};

struct jive_three_address_code *
jive_assignment_code_create(struct jive_basic_block * basic_block,
	struct jive_variable_code * variable, jive_three_address_code * tac);

static inline struct jive_variable_code *
jive_assignment_code_get_variable(const struct jive_assignment_code * self)
{
	return static_cast<jive_variable_code*>(self->operands[0]);
}

static inline struct jive_three_address_code *
jive_assignment_code_get_rhs(const struct jive_assignment_code * self)
{
	return self->operands[1];
}

#endif
