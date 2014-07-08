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

namespace jive {
namespace frontend {

call_code::~call_code() noexcept {}

call_code::call_code(jive_basic_block * basic_block, struct jive_clg_node * callee_,
	std::vector<three_address_code*> & arguments)
	: three_address_code(basic_block, arguments)
	, callee(callee_)
{}

std::string
call_code::debug_string() const
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

}
}

jive::frontend::three_address_code *
jive_call_code_create(jive_basic_block * basic_block, jive_clg_node * callee,
	size_t narguments, jive::frontend::three_address_code * const arguments[])
{
	std::vector<jive::frontend::three_address_code*> args(arguments, arguments+narguments);
	return new jive::frontend::call_code(basic_block, callee, args);
}
