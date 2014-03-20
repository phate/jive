/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_TAC_THREE_ADDRESS_CODE_PRIVATE_H
#define JIVE_FRONTEND_TAC_THREE_ADDRESS_CODE_PRIVATE_H

#include <stdlib.h>

struct jive_basic_block;
struct jive_three_address_code;
struct jive_three_address_code_attrs;

void
jive_three_address_code_init_(struct jive_three_address_code * self,
	struct jive_basic_block * basic_block,
	size_t noperands, struct jive_three_address_code * const operands[]);

void
jive_three_address_code_fini_(struct jive_three_address_code * self);

void
jive_three_address_code_get_label_(const struct jive_three_address_code * self,
	struct jive_buffer * buffer);

const struct jive_three_address_code_attrs *
jive_three_address_code_get_attrs_(const struct jive_three_address_code * self);

struct jive_three_address_code *
jive_three_address_code_create_(struct jive_basic_block * basic_block,
	const struct jive_three_address_code_attrs * attrs,
	size_t noperands, struct jive_three_address_code * const operands[]);

#endif
