/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/frontend/basic_block.h>
#include <jive/frontend/cfg.h>
#include <jive/frontend/tac/reference.h>
#include <jive/frontend/tac/variable.h>

#include <stdio.h>

namespace jive {
namespace frontend {

reference_code::~reference_code() noexcept {}

reference_code::reference_code(jive_basic_block * basic_block, variable_code * variable)
	: three_address_code(basic_block, {variable})
{}

std::string
reference_code::debug_string() const
{
	std::string label("ref ");
	if (dynamic_cast<variable_code*>(operands[0]) != NULL)
		label.append(operands[0]->debug_string());
	else {
		char tmp[32];
		snprintf(tmp, sizeof(tmp), "%p", operands[0]);
		label.append(tmp);
	}
	return label;
}

}
}

jive::frontend::three_address_code *
jive_reference_code_create(jive_basic_block * basic_block, jive::frontend::variable_code * variable)
{
	return new jive::frontend::reference_code(basic_block, variable);
}
