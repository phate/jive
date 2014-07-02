/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/frontend/basic_block.h>
#include <jive/frontend/cfg.h>
#include <jive/frontend/tac/bitstring/bitconstant.h>
#include <jive/frontend/tac/three_address_code-private.h>
#include <jive/frontend/clg.h>
#include <jive/frontend/clg_node.h>
#include <jive/util/buffer.h>

#include <string.h>

jive_bitconstant_code::~jive_bitconstant_code() noexcept {}

std::string
jive_bitconstant_code::debug_string() const
{
	std::string label;
	for (size_t i = 0; i < attrs.bits.size(); i++)
		label.append(&attrs.bits[i]);

	return label;
}

static void
jive_bitconstant_code_get_label_(const struct jive_three_address_code * self,
	struct jive_buffer * buffer);

static const struct jive_three_address_code_attrs *
jive_bitconstant_code_get_attrs_(const struct jive_three_address_code * self);

const struct jive_three_address_code_class JIVE_BITCONSTANT_CODE = {
	parent : &JIVE_THREE_ADDRESS_CODE,
	name : "BITCONSTANT",
	fini : nullptr, /* override */
	get_label : jive_bitconstant_code_get_label_, /* override */
	get_attrs : jive_bitconstant_code_get_attrs_, /* override */
};

static void
jive_bitconstant_code_init_(struct jive_bitconstant_code * self,
	struct jive_basic_block * basic_block, size_t nbits, const char * bits)
{
	jive_three_address_code_init_(self, basic_block, 0, NULL);
	self->attrs.bits.resize(nbits);
	for (size_t i = 0; i < nbits; i++)
		self->attrs.bits[nbits-i-1] = bits[nbits-i-1];
}

static void
jive_bitconstant_code_get_label_(const struct jive_three_address_code * self_,
	struct jive_buffer * buffer)
{
	struct jive_bitconstant_code * self = (struct jive_bitconstant_code *)self_;

	size_t i;
	for (i = 0; i < self->attrs.bits.size(); i++)
		jive_buffer_putbyte(buffer, self->attrs.bits[i]);
}

static const struct jive_three_address_code_attrs *
jive_bitconstant_code_get_attrs_(const struct jive_three_address_code * self_)
{
	struct jive_bitconstant_code * self = (struct jive_bitconstant_code *)self_;
	return &self->attrs.base;
}

jive_three_address_code *
jive_bitconstant_code_create(struct jive_basic_block * basic_block, size_t nbits, const char * bits)
{
	jive_bitconstant_code * bitconstant = new jive_bitconstant_code;
	bitconstant->class_ = &JIVE_BITCONSTANT_CODE;
	jive_bitconstant_code_init_(bitconstant, basic_block, nbits, bits);
	return bitconstant;
}
