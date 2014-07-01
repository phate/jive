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

static void
jive_bitconstant_code_fini_(struct jive_three_address_code * self);

static void
jive_bitconstant_code_get_label_(const struct jive_three_address_code * self,
	struct jive_buffer * buffer);

static const struct jive_three_address_code_attrs *
jive_bitconstant_code_get_attrs_(const struct jive_three_address_code * self);

static struct jive_three_address_code *
jive_bitconstant_code_create_(struct jive_basic_block * basic_block,
	const jive_three_address_code_attrs * attrs,
	size_t noperands, struct jive_three_address_code * const operands[]);

const struct jive_three_address_code_class JIVE_BITCONSTANT_CODE = {
	parent : &JIVE_THREE_ADDRESS_CODE,
	name : "BITCONSTANT",
	fini : jive_bitconstant_code_fini_, /* override */
	get_label : jive_bitconstant_code_get_label_, /* override */
	get_attrs : jive_bitconstant_code_get_attrs_, /* override */
	create : jive_bitconstant_code_create_ /* override */
};

static void
jive_bitconstant_code_init_(struct jive_bitconstant_code * self,
	struct jive_basic_block * basic_block, std::vector<char> bits)
{
	jive_three_address_code_init_(self, basic_block, 0, NULL);
	self->attrs.bits = bits;
}

static void
jive_bitconstant_code_fini_(struct jive_three_address_code * self_)
{
	struct jive_bitconstant_code * self = (struct jive_bitconstant_code *)self_;
	jive_three_address_code_fini_(self_);
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

static jive_three_address_code *
jive_bitconstant_code_create_(struct jive_basic_block * basic_block,
	const jive_three_address_code_attrs * attrs_,
	size_t noperands, struct jive_three_address_code * const operands[])
{
	jive_bitconstant_code_attrs * attrs = (jive_bitconstant_code_attrs *)attrs_;

	jive_bitconstant_code * bitconstant = new jive_bitconstant_code;
	bitconstant->class_ = &JIVE_BITCONSTANT_CODE;
	jive_bitconstant_code_init_(bitconstant, basic_block, attrs->bits);
	return bitconstant;
}

jive_three_address_code *
jive_bitconstant_code_create(struct jive_basic_block * basic_block, size_t nbits, const char * bits)
{
	jive_bitconstant_code_attrs attrs;

	attrs.bits.resize(nbits);
	for (size_t i = 0; i < nbits; i++)
		attrs.bits[nbits-i-1] = bits[nbits-i-1];

	return jive_bitconstant_code_create_(basic_block, &attrs.base, 0, NULL);
}
