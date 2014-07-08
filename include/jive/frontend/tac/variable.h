/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_TAC_VARIABLE_H
#define JIVE_FRONTEND_TAC_VARIABLE_H

#include <jive/frontend/tac/three_address_code.h>

#include <string>

class jive_basic_block;

namespace jive {
namespace frontend {

class variable_code final : public three_address_code {
public:
	virtual ~variable_code() noexcept;

	variable_code(jive_basic_block * basic_block, const char * name);

	virtual std::string debug_string() const override;

	std::string name;
};

}
}

jive::frontend::three_address_code *
jive_variable_code_create(jive_basic_block * basic_block, const char * name);

#endif
