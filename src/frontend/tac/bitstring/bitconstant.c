/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/frontend/basic_block.h>
#include <jive/frontend/cfg.h>
#include <jive/frontend/tac/bitstring/bitconstant.h>
#include <jive/frontend/clg.h>
#include <jive/frontend/clg_node.h>

#include <string.h>

namespace jive {
namespace frontend {

bitconstant_code::~bitconstant_code() noexcept {}

bitconstant_code::bitconstant_code(jive_basic_block * basic_block, size_t nbits, const char * bits_)
	: three_address_code(basic_block, {})
{
	bits.resize(nbits);
	for (size_t i = 0; i < nbits; i++)
		bits[nbits-i-1] = bits_[nbits-i-1];
}

std::string
bitconstant_code::debug_string() const
{
	std::string label;
	for (size_t i = 0; i < bits.size(); i++)
		label.append(&bits[i]);

	return label;
}

}
}

jive::frontend::three_address_code *
jive_bitconstant_code_create(jive_basic_block * basic_block, size_t nbits, const char * bits)
{
	return new jive::frontend::bitconstant_code(basic_block, nbits, bits);
}
