/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/frontend/basic_block.h>
#include <jive/frontend/cfg.h>
#include <jive/frontend/tac/three_address_code-private.h>
#include <jive/frontend/tac/three_address_code.h>
#include <jive/util/buffer.h>
#include <jive/util/list.h>

#include <string.h>

const struct jive_three_address_code_class JIVE_THREE_ADDRESS_CODE = {
	parent : 0,
	name : "THREE_ADDRESS_CODE",
	fini : jive_three_address_code_fini_,
	get_label : jive_three_address_code_get_label_,
	get_attrs : jive_three_address_code_get_attrs_,
	create : jive_three_address_code_create_
};

void
jive_three_address_code_init_(struct jive_three_address_code * self,
	struct jive_basic_block * basic_block,
	size_t noperands, struct jive_three_address_code * const operands[])
{
	self->basic_block = basic_block;
	self->basic_block_three_address_codes_list.prev = NULL;
	self->basic_block_three_address_codes_list.next = NULL;

	size_t i;
	self->noperands = noperands;
	self->operands = jive_context_malloc(basic_block->base.cfg->context,
		sizeof(jive_three_address_code *) * self->noperands);
	for (i = 0; i < noperands; i++)
		self->operands[i] = operands[i];

	JIVE_LIST_PUSH_BACK(basic_block->three_address_codes, self, basic_block_three_address_codes_list);
}

void
jive_three_address_code_fini_(struct jive_three_address_code * self)
{
	JIVE_LIST_REMOVE(self->basic_block->three_address_codes, self,
		basic_block_three_address_codes_list);
	jive_context_free(self->basic_block->base.cfg->context, self->operands);

	self->noperands = 0;

	self->basic_block_three_address_codes_list.prev = 0;
	self->basic_block_three_address_codes_list.next = 0;
}

void
jive_three_address_code_get_label_(const jive_three_address_code * self, struct jive_buffer * buffer)
{
	jive_buffer_putstr(buffer, self->class_->name);
}

const jive_three_address_code_attrs *
jive_three_address_code_get_attrs_(const jive_three_address_code * self)
{
	return 0;
}

struct jive_three_address_code *
jive_three_address_code_create_(struct jive_basic_block * basic_block,
	const jive_three_address_code_attrs * attrs,
	size_t noperands, struct jive_three_address_code * const operands[])
{
	jive_three_address_code * three_address_code = new jive_three_address_code;
	three_address_code->class_ = &JIVE_THREE_ADDRESS_CODE;
	jive_three_address_code_init_(three_address_code, basic_block, noperands, operands);

	return three_address_code;
}

void
jive_three_address_code_destroy(struct jive_three_address_code * self)
{
	self->class_->fini(self);
	delete self;
}
