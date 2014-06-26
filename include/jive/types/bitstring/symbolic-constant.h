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
namespace bits {

struct symbolicconstant_operation : public base::unary_op {
	size_t nbits;
	std::string name;
};

}
}

typedef jive::operation_node<jive::bits::symbolicconstant_operation>
	jive_bitsymbolicconstant_node;

/**
	\brief Create symbolic constant
	\param graph Graph to create constant in
	\param nbits Number of bits
	\param name Symbol name
	\returns Bitstring value representing constant
	
	Create new bitconstant node.
*/
jive_node *
jive_bitsymbolicconstant_create(struct jive_graph * graph, size_t nbits, const char * name);

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
jive_bitsymbolicconstant(struct jive_graph * graph, size_t nbits, const char * name);

#endif
