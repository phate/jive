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

static void
jive_variable_code_fini_(struct jive_three_address_code * self);

static void
jive_variable_code_get_label_(const struct jive_three_address_code * self,
	struct jive_buffer * buffer);

static const struct jive_three_address_code_attrs *
jive_variable_code_get_attrs_(const struct jive_three_address_code * self);

static struct jive_three_address_code *
jive_variable_code_create_(struct jive_basic_block * basic_block,
	const struct jive_three_address_code_attrs * attrs,
	size_t noperands, struct jive_three_address_code * const operands[]);

const struct jive_three_address_code_class JIVE_VARIABLE_CODE = {
	parent : &JIVE_THREE_ADDRESS_CODE,
	name : "VARIABLE",
	fini : jive_variable_code_fini_, /* override */
	get_label : jive_variable_code_get_label_, /* override */
	get_attrs : jive_variable_code_get_attrs_, /* override */
	create : jive_variable_code_create_ /* override */
};

static void
jive_variable_code_init_(struct jive_variable_code * self,
	struct jive_basic_block * basic_block, std::string & name)
{
	jive_three_address_code_init_(self, basic_block, 0, NULL);
	self->attrs.name = name;
}

static void
jive_variable_code_fini_(struct jive_three_address_code * self_)
{
	struct jive_variable_code * self = (struct jive_variable_code *)self_;
	jive_three_address_code_fini_(self_);
}

static void
jive_variable_code_get_label_(const struct jive_three_address_code * self_,
	struct jive_buffer * buffer)
{
	struct jive_variable_code * self = (struct jive_variable_code *)self_;
	jive_buffer_putstr(buffer, self->attrs.name.c_str());
}

static const struct jive_three_address_code_attrs *
jive_variable_code_get_attrs_(const struct jive_three_address_code * self_)
{
	struct jive_variable_code * self = (struct jive_variable_code *)self_;
	return &self->attrs.base;
}

static jive_three_address_code *
jive_variable_code_create_(struct jive_basic_block * basic_block,
	const jive_three_address_code_attrs * attrs_,
	size_t noperands, struct jive_three_address_code * const operands[])
{
	jive_variable_code_attrs * attrs = (jive_variable_code_attrs *)attrs_;

	jive_variable_code * variable = new jive_variable_code;
	variable->class_ = &JIVE_VARIABLE_CODE;
	jive_variable_code_init_(variable, basic_block, attrs->name);
	return variable;
}

jive_three_address_code *
jive_variable_code_create(struct jive_basic_block * basic_block, const char * name)
{
	jive_variable_code_attrs attrs;
	attrs.name = (char *)name;

	return jive_variable_code_create_(basic_block, &attrs.base, 0, NULL);
}
