/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/constant.h>

#include <string.h>

#include <jive/types/bitstring/type.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/operators.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace base {
// explicit instantiation
template class domain_const_op<
	bits::type, bits::value_repr, bits::format_value, bits::type_of_value
>;
}
}

jive::output *
jive_bitconstant(jive_graph * graph, size_t nbits, const char bits[])
{
	jive::bits::constant_op op(std::vector<char>(bits, bits + nbits));
	return jive_node_create_normalized(graph, op, {})[0];
}

jive::output *
jive_bitconstant_unsigned(struct jive_graph * graph, size_t nbits, uint64_t value)
{
	char bits[nbits];
	jive_bitstring_init_unsigned(bits, nbits, value);
	
	jive::bits::constant_op op(std::vector<char>(bits, bits + nbits));
	return jive_node_create_normalized(graph, op, {})[0];
}

jive::output *
jive_bitconstant_signed(struct jive_graph * graph, size_t nbits, int64_t value)
{
	char bits[nbits];
	jive_bitstring_init_signed(bits, nbits, value);

	jive::bits::constant_op op(std::vector<char>(bits, bits + nbits));
	return jive_node_create_normalized(graph, op, {})[0];
}
