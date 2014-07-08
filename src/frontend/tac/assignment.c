/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/frontend/basic_block.h>
#include <jive/frontend/cfg.h>
#include <jive/frontend/tac/assignment.h>

#include <stdio.h>

namespace jive {
namespace frontend {

assignment_code::~assignment_code() noexcept {}

assignment_code::assignment_code(jive_basic_block * basic_block,
	variable_code * variable, three_address_code * tac)
	: three_address_code(basic_block, {variable, tac})
{}

std::string
assignment_code::debug_string() const
{
	std::string label = operands[0]->debug_string();
	label.append(" := ");

	char tmp[32];
	snprintf(tmp, sizeof(tmp), "%p", operands[1]);
	label.append(tmp);
	return label;
}

}
}

jive::frontend::three_address_code *
jive_assignment_code_create(jive_basic_block * basic_block, jive::frontend::variable_code * variable,
	jive::frontend::three_address_code * tac)
{
	return new jive::frontend::assignment_code(basic_block, variable, tac);
}
