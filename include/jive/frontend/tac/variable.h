/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_TAC_VARIABLE_H
#define JIVE_FRONTEND_TAC_VARIABLE_H

#include <jive/frontend/tac/three_address_code.h>

#include <string>

extern const jive_three_address_code_class JIVE_VARIABLE_CODE;

class jive_variable_code final : public jive_three_address_code {
public:
	virtual ~jive_variable_code() noexcept;

	virtual std::string debug_string() const override;

	std::string name;
};

struct jive_three_address_code *
jive_variable_code_create(struct jive_basic_block * basic_block, const char * name);

#endif
