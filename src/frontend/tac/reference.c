/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/context.h>
#include <jive/frontend/basic_block.h>
#include <jive/frontend/cfg.h>
#include <jive/frontend/tac/reference.h>
#include <jive/frontend/tac/three_address_code-private.h>
#include <jive/frontend/tac/variable.h>
#include <jive/util/buffer.h>

#include <stdio.h>

static void
jive_reference_code_get_label_(const struct jive_three_address_code * self,
	struct jive_buffer * buffer);

static struct jive_three_address_code *
jive_reference_code_create_(struct jive_basic_block * basic_block,
	const struct jive_three_address_code_attrs * attrs,
	size_t noperands, struct jive_three_address_code * const operands[]);

const struct jive_three_address_code_class JIVE_REFERENCE_CODE = {
	.parent = &JIVE_THREE_ADDRESS_CODE,
	.name = "REFERENCE",
	.fini = jive_three_address_code_fini_, /* inherit */
	.get_label = jive_reference_code_get_label_, /* override */
	.get_attrs = jive_three_address_code_get_attrs_, /* inherit */
	.create = jive_reference_code_create_ /* override */
};

static void
jive_reference_code_init_(jive_reference_code * self, struct jive_basic_block * basic_block,
	struct jive_variable_code * variable)
{
	jive_three_address_code_init_(&self->base, basic_block,
		1, (jive_three_address_code *[]){&variable->base});
}

static void
jive_reference_code_get_label_(const struct jive_three_address_code * self,
	struct jive_buffer * buffer)
{
	jive_buffer_putstr(buffer, "ref ");
	if (jive_variable_code_cast(self->operands[0]) != NULL)
		jive_three_address_code_get_label(self->operands[0], buffer);
	else {
		char tmp[32];
		snprintf(tmp, sizeof(tmp), "%p", self->operands[0]);
		jive_buffer_putstr(buffer, tmp);
	}
}

static jive_three_address_code *
jive_reference_code_create_(struct jive_basic_block * basic_block,
	const jive_three_address_code_attrs * attrs,
	size_t noperands, struct jive_three_address_code * const operands[])
{
	jive_reference_code * ref = jive_context_malloc(basic_block->base.cfg->context, sizeof(*ref));
	ref->base.class_ = &JIVE_REFERENCE_CODE;
	jive_reference_code_init_(ref, basic_block, jive_variable_code_cast(operands[0]));
	return &ref->base;
}

jive_three_address_code *
jive_reference_code_create(struct jive_basic_block * basic_block, jive_variable_code * variable)
{
	return jive_reference_code_create_(basic_block, NULL,
		1, (jive_three_address_code *[]){&variable->base});
}
