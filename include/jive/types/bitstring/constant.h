/*
 * Copyright 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_CONSTANT_H
#define JIVE_TYPES_BITSTRING_CONSTANT_H

#include <stdint.h>
#include <vector>

#include <jive/types/bitstring/type.h>
#include <jive/types/bitstring/value-representation.h>
#include <jive/util/bitstring.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/operators.h>

extern const jive_node_class JIVE_BITCONSTANT_NODE;

namespace jive {
namespace bits {

struct type_of_value {
	type operator()(const value_repr & repr) const
	{
		return type(repr.size());
	}
};

struct format_value {
	std::string operator()(const value_repr & repr) const
	{
		return std::string(repr.rbegin(), repr.rend());
	}
};

typedef base::domain_const_op<
	&JIVE_BITCONSTANT_NODE, type, value_repr, format_value, type_of_value
> constant_op;

inline constant_op
uint_constant_op(size_t nbits, uint64_t value)
{
	return constant_op(value_repr_from_uint(nbits, value));
}

inline constant_op
int_constant_op(size_t nbits, int64_t value)
{
	return constant_op(value_repr_from_int(nbits, value));
}

}

namespace base {
// declare explicit instantiation
extern template class domain_const_op<
	&JIVE_BITCONSTANT_NODE, bits::type, bits::value_repr, bits::format_value, bits::type_of_value
>;
}

}

typedef jive::operation_node<jive::bits::constant_op> jive_bitconstant_node;

/**
	\brief Create bitconstant
	\param graph Graph to create constant in
	\param nbits Number of bits
	\param bits Values of bits
	\returns Bitstring value representing constant
	
	Convenience function that either creates a new constant or
	returns the output handle of an existing constant.
*/
jive::output *
jive_bitconstant(struct jive_graph * graph, size_t nbits, const char bits[]);

jive::output *
jive_bitconstant_unsigned(struct jive_graph * graph, size_t nbits, uint64_t value);

jive::output *
jive_bitconstant_signed(struct jive_graph * graph, size_t nbits, int64_t value);

JIVE_EXPORTED_INLINE jive::output *
jive_bitconstant_undefined(struct jive_graph * graph, size_t nbits)
{
	size_t i;
	char bits[nbits];
	for (i = 0; i < nbits; i++)
		bits[i] = 'X';

	return jive_bitconstant(graph, nbits, bits);
}

JIVE_EXPORTED_INLINE jive::output *
jive_bitconstant_defined(struct jive_graph * graph, size_t nbits)
{
	size_t i;
	char bits[nbits];
	for (i = 0; i < nbits; i++)
		bits[i] = 'D';

	return jive_bitconstant(graph, nbits, bits);
}

JIVE_EXPORTED_INLINE bool
jive_bitconstant_is_zero(const jive_bitconstant_node * node)
{
	for (const char bit : node->operation().value()) {
		if (bit != '0') return false;
	}
	return true;
}

JIVE_EXPORTED_INLINE bool
jive_bitconstant_is_one(const jive_bitconstant_node * node)
{
	char expect = '1';
	for (const char bit : node->operation().value()) {
		if (bit != expect) return false;
		expect = '0';
	}
	return true;
}

JIVE_EXPORTED_INLINE bool
jive_bitconstant_is_minus_one(const jive_bitconstant_node * node)
{
	for (const char bit : node->operation().value()) {
		if (bit != '1') return false;
	}
	return true;
}

JIVE_EXPORTED_INLINE bool
jive_bitconstant_equals_signed(const jive_bitconstant_node * node, int64_t value)
{
	return jive_bitstring_equals_signed(
		&node->operation().value()[0], node->operation().value().size(), value);
}

JIVE_EXPORTED_INLINE bool
jive_bitconstant_equals_unsigned(const jive_bitconstant_node * node, uint64_t value)
{
	return jive_bitstring_equals_unsigned(
		&node->operation().value()[0], node->operation().value().size(), value);
}

JIVE_EXPORTED_INLINE uint64_t
jive_bitconstant_node_to_unsigned(const jive_bitconstant_node * node)
{
	return jive_bitstring_to_unsigned(
		&node->operation().value()[0], node->operation().value().size());
}

JIVE_EXPORTED_INLINE int64_t
jive_bitconstant_node_to_signed(const jive_bitconstant_node * node)
{
	return jive_bitstring_to_signed(
		&node->operation().value()[0], node->operation().value().size());
}

#endif
