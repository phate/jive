/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/frontend/basic_block.h>
#include <jive/frontend/cfg.h>
#include <jive/frontend/tac/reference.h>
#include <jive/frontend/tac/three_address_code-private.h>
#include <jive/frontend/tac/variable.h>
#include <jive/util/buffer.h>

#include <stdio.h>

jive_reference_code::~jive_reference_code() noexcept {}

std::string
jive_reference_code::debug_string() const
{
	std::string label("ref ");
	if (dynamic_cast<jive_variable_code*>(operands[0]) != NULL)
		label.append(operands[0]->debug_string());
	else {
		char tmp[32];
		snprintf(tmp, sizeof(tmp), "%p", operands[0]);
		label.append(tmp);
	}
	return label;
}

const struct jive_three_address_code_class JIVE_REFERENCE_CODE = {
	parent : &JIVE_THREE_ADDRESS_CODE,
	name : "REFERENCE",
	fini : nullptr, /* inherit */
	get_attrs : jive_three_address_code_get_attrs_, /* inherit */
};

static void
jive_reference_code_init_(jive_reference_code * self, struct jive_basic_block * basic_block,
	struct jive_variable_code * variable)
{
	jive_three_address_code * tmparray0[] = {variable};
	jive_three_address_code_init_(self, basic_block, 1, tmparray0);
}

jive_three_address_code *
jive_reference_code_create(struct jive_basic_block * basic_block, jive_variable_code * variable)
{
	jive_reference_code * ref = new jive_reference_code;
	ref->class_ = &JIVE_REFERENCE_CODE;
	jive_reference_code_init_(ref, basic_block, variable);
	return ref;
}
