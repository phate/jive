/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/frontend/basic_block.h>
#include <jive/frontend/cfg.h>
#include <jive/frontend/tac/variable.h>
#include <jive/frontend/tac/three_address_code-private.h>
#include <jive/util/buffer.h>

#include <string.h>


jive_variable_code::~jive_variable_code() noexcept {}

std::string
jive_variable_code::debug_string() const
{
	return attrs.name;
}

static const struct jive_three_address_code_attrs *
jive_variable_code_get_attrs_(const struct jive_three_address_code * self);

const struct jive_three_address_code_class JIVE_VARIABLE_CODE = {
	parent : &JIVE_THREE_ADDRESS_CODE,
	name : "VARIABLE",
	fini : nullptr, /* override */
	get_attrs : jive_variable_code_get_attrs_, /* override */
};

static void
jive_variable_code_init_(struct jive_variable_code * self,
	struct jive_basic_block * basic_block, const char * name)
{
	jive_three_address_code_init_(self, basic_block, 0, NULL);
	self->attrs.name = name;
}

static const struct jive_three_address_code_attrs *
jive_variable_code_get_attrs_(const struct jive_three_address_code * self_)
{
	struct jive_variable_code * self = (struct jive_variable_code *)self_;
	return &self->attrs.base;
}

jive_three_address_code *
jive_variable_code_create(struct jive_basic_block * basic_block, const char * name)
{
	jive_variable_code * variable = new jive_variable_code;
	variable->class_ = &JIVE_VARIABLE_CODE;
	jive_variable_code_init_(variable, basic_block, name);
	return variable;
}
