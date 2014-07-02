/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/frontend/basic_block.h>
#include <jive/frontend/cfg.h>
#include <jive/frontend/tac/assignment.h>
#include <jive/frontend/tac/three_address_code-private.h>
#include <jive/util/buffer.h>

#include <stdio.h>

jive_assignment_code::~jive_assignment_code() noexcept {}

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

const struct jive_three_address_code_class JIVE_ASSIGNMENT_CODE = {
	parent : &JIVE_THREE_ADDRESS_CODE,
	name : "ASSIGNMENT",
	fini : nullptr, /* inherit */
};

static void
jive_assignment_code_init_(jive_assignment_code * self, struct jive_basic_block * basic_block,
	struct jive_variable_code * variable, struct jive_three_address_code * tac)
{
	jive_three_address_code * tmparray0[] = {variable, tac};
	jive_three_address_code_init_(self, basic_block, 2, tmparray0);
}

jive_three_address_code *
jive_assignment_code_create(struct jive_basic_block * basic_block, jive_variable_code * variable,
	jive_three_address_code * tac)
{
	jive_assignment_code * ass = new jive_assignment_code;
	ass->class_ = &JIVE_ASSIGNMENT_CODE;
	jive_assignment_code_init_(ass, basic_block, variable, tac);
	return ass;
}
