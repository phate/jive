/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/frontend/basic_block.h>
#include <jive/frontend/cfg.h>
#include <jive/frontend/tac/variable.h>

#include <string.h>


jive_variable_code::~jive_variable_code() noexcept {}

jive_variable_code::jive_variable_code(struct jive_basic_block * basic_block, const char * name_)
	:	jive_three_address_code(basic_block, {})
	, name(name_)
{}

std::string
jive_variable_code::debug_string() const
{
	return name;
}

jive_three_address_code *
jive_variable_code_create(struct jive_basic_block * basic_block, const char * name)
{
	return new jive_variable_code(basic_block, name);
}
