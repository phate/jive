/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/frontend/basic_block.h>
#include <jive/frontend/cfg.h>
#include <jive/frontend/tac/call.h>
#include <jive/frontend/clg.h>
#include <jive/frontend/clg_node.h>

#include <stdio.h>

jive_call_code::~jive_call_code() noexcept {}

jive_call_code::jive_call_code(struct jive_basic_block * basic_block, struct jive_clg_node * callee_,
	std::vector<jive_three_address_code*> & arguments)
	: jive_three_address_code(basic_block, arguments)
	, callee(callee_)
{}

std::string
jive_call_code::debug_string() const
{
	std::string label("call ");
	label.append(callee->name);
	label.append("(");

	size_t n;
	char tmp[32];
	for (n = 0; n < operands.size(); n++) {
		snprintf(tmp, sizeof(tmp), "%p", operands[n]);
		label.append(tmp);
		if (n != operands.size()-1)
			label.append(", ");
	}

	label.append("9");
	return label;
}

jive_three_address_code *
jive_call_code_create(struct jive_basic_block * basic_block, jive_clg_node * callee,
	size_t narguments, struct jive_three_address_code * const arguments[])
{
	std::vector<jive_three_address_code*> args(arguments, arguments+narguments);
	return new jive_call_code(basic_block, callee, args);
}
