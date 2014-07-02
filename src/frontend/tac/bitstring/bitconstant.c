/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/frontend/basic_block.h>
#include <jive/frontend/cfg.h>
#include <jive/frontend/tac/bitstring/bitconstant.h>
#include <jive/frontend/clg.h>
#include <jive/frontend/clg_node.h>
#include <jive/util/buffer.h>

#include <string.h>

jive_bitconstant_code::~jive_bitconstant_code() noexcept {}

jive_bitconstant_code::jive_bitconstant_code(struct jive_basic_block * basic_block, size_t nbits,
	const char * bits_)
	: jive_three_address_code(basic_block, {})
{
	bits.resize(nbits);
	for (size_t i = 0; i < nbits; i++)
		bits[nbits-i-1] = bits_[nbits-i-1];
}

std::string
jive_bitconstant_code::debug_string() const
{
	std::string label;
	for (size_t i = 0; i < bits.size(); i++)
		label.append(&bits[i]);

	return label;
}

jive_three_address_code *
jive_bitconstant_code_create(struct jive_basic_block * basic_block, size_t nbits, const char * bits)
{
	return new jive_bitconstant_code(basic_block, nbits, bits);
}
