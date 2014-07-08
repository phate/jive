/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_TAC_REFERENCE_H
#define JIVE_FRONTEND_TAC_REFERENCE_H

#include <jive/frontend/tac/three_address_code.h>

class jive_basic_block;

namespace jive {
namespace frontend {

class variable_code;

class reference_code final : public three_address_code {
public:
	virtual ~reference_code() noexcept;

	reference_code(jive_basic_block * basic_block, variable_code * variable);

	virtual std::string debug_string() const override;
};

}
}

jive::frontend::three_address_code *
jive_reference_code_create(jive_basic_block * basic_block, jive::frontend::variable_code * variable);

static inline jive::frontend::three_address_code *
jive_reference_code_get_reference(const jive::frontend::reference_code * self)
{
	return self->operands[0];
}

static inline void
jive_reference_code_set_reference(jive::frontend::reference_code * self,
	jive::frontend::three_address_code * tac)
{
	self->operands[0] = tac;
}

#endif
