/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_TAC_ASSIGNMENT_H
#define JIVE_FRONTEND_TAC_ASSIGNMENT_H

#include <jive/frontend/tac/three_address_code.h>
#include <jive/frontend/tac/variable.h>

class jive_basic_block;

namespace jive {
namespace frontend {

class assignment_code final : public three_address_code {
public:
	virtual ~assignment_code() noexcept;

	assignment_code(jive_basic_block * basic_block, variable_code * variable,
		three_address_code * tac);

	virtual std::string debug_string() const override;
};

}
}

jive::frontend::three_address_code *
jive_assignment_code_create(jive_basic_block * basic_block,
	jive::frontend::variable_code * variable, jive::frontend::three_address_code * tac);

static inline jive::frontend::variable_code *
jive_assignment_code_get_variable(const jive::frontend::assignment_code * self)
{
	return static_cast<jive::frontend::variable_code*>(self->operands[0]);
}

static inline jive::frontend::three_address_code *
jive_assignment_code_get_rhs(const jive::frontend::assignment_code * self)
{
	return self->operands[1];
}

#endif
