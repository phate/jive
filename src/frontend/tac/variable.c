/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/frontend/basic_block.h>
#include <jive/frontend/cfg.h>
#include <jive/frontend/tac/variable.h>

#include <string.h>

namespace jive {
namespace frontend {

variable_code::~variable_code() noexcept {}

variable_code::variable_code(jive_basic_block * basic_block, const char * name_)
	:	three_address_code(basic_block, {})
	, name(name_)
{}

std::string
variable_code::debug_string() const
{
	return name;
}

}
}

jive::frontend::three_address_code *
jive_variable_code_create(jive_basic_block * basic_block, const char * name)
{
	return new jive::frontend::variable_code(basic_block, name);
}
