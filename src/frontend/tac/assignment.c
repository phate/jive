/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/frontend/basic_block.h>
#include <jive/frontend/cfg.h>
#include <jive/frontend/tac/assignment.h>

#include <stdio.h>

jive_assignment_code::~jive_assignment_code() noexcept {}

jive_assignment_code::jive_assignment_code(struct jive_basic_block * basic_block,
	struct jive_variable_code * variable, jive_three_address_code * tac)
	: jive_three_address_code(basic_block, {variable, tac})
{}

std::string
jive_assignment_code::debug_string() const
{
	std::string label = operands[0]->debug_string();
	label.append(" := ");

	char tmp[32];
	snprintf(tmp, sizeof(tmp), "%p", operands[1]);
	label.append(tmp);
	return label;
}

jive_three_address_code *
jive_assignment_code_create(struct jive_basic_block * basic_block, jive_variable_code * variable,
	jive_three_address_code * tac)
{
	return new jive_assignment_code(basic_block, variable, tac);
}
