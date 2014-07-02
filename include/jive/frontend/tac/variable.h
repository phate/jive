/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_TAC_VARIABLE_H
#define JIVE_FRONTEND_TAC_VARIABLE_H

#include <jive/frontend/tac/three_address_code.h>

#include <string>

extern const jive_three_address_code_class JIVE_VARIABLE_CODE;

typedef struct jive_variable_code_attrs jive_variable_code_attrs;

struct jive_variable_code_attrs {
	jive_three_address_code_attrs base;
	std::string name;
};

class jive_variable_code final : public jive_three_address_code {
public:
	virtual ~jive_variable_code() noexcept;

	jive_variable_code_attrs attrs;
};

struct jive_three_address_code *
jive_variable_code_create(struct jive_basic_block * basic_block, const char * name);

#endif
