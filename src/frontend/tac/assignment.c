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

static void
jive_assignment_code_get_label_(const struct jive_three_address_code * self,
	struct jive_buffer * buffer);

const struct jive_three_address_code_class JIVE_ASSIGNMENT_CODE = {
	parent : &JIVE_THREE_ADDRESS_CODE,
	name : "ASSIGNMENT",
	fini : nullptr, /* inherit */
	get_label : jive_assignment_code_get_label_, /* override */
	get_attrs : jive_three_address_code_get_attrs_, /* inherit */
};

static void
jive_assignment_code_init_(jive_assignment_code * self, struct jive_basic_block * basic_block,
	struct jive_variable_code * variable, struct jive_three_address_code * tac)
{
	jive_three_address_code * tmparray0[] = {variable, tac};
	jive_three_address_code_init_(self, basic_block, 2, tmparray0);
}

static void
jive_assignment_code_get_label_(const struct jive_three_address_code * self,
	struct jive_buffer * buffer)
{
	jive_three_address_code_get_label(self->operands[0], buffer);
	jive_buffer_putstr(buffer, " := ");

	char tmp[32];
	snprintf(tmp, sizeof(tmp), "%p", self->operands[1]);
	jive_buffer_putstr(buffer, tmp);
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
