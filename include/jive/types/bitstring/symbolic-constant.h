/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_SYMBOLIC_CONSTANT_H
#define JIVE_TYPES_BITSTRING_SYMBOLIC_CONSTANT_H

#include <string>

#include <jive/types/bitstring/type.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/operators.h>

extern const jive_node_class JIVE_BITSYMBOLICCONSTANT_NODE;

namespace jive {
namespace base {
// declare explicit instantiation
extern template class domain_symbol_op<&JIVE_BITSYMBOLICCONSTANT_NODE, jive::bits::type>;
}

namespace bits {
typedef base::domain_symbol_op<&JIVE_BITSYMBOLICCONSTANT_NODE, jive::bits::type>
	symbol_op;
}
}

/**
	\brief Create symbolic constant
	\param graph Graph to create constant in
	\param nbits Number of bits
	\param name Symbol name
	\returns Bitstring value representing constant
	
	Convenience function that either creates a new constant or
	returns the output handle of an existing constant.
*/
jive::output *
jive_bitsymbolicconstant(jive_graph * graph, size_t nbits, const char * name);

#endif
