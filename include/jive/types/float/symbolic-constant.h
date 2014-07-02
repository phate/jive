/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_SYMBOLIC_CONSTANT_H
#define JIVE_TYPES_FLOAT_SYMBOLIC_CONSTANT_H

#include <string>

#include <jive/types/float/flttype.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/operators.h>

extern const jive_node_class JIVE_FLTSYMBOLICCONSTANT_NODE;

namespace jive {
namespace base {
// declare explicit instantiation
extern template class domain_symbol_op<&JIVE_FLTSYMBOLICCONSTANT_NODE, jive::flt::type>;
}

namespace flt {
typedef base::domain_symbol_op<&JIVE_FLTSYMBOLICCONSTANT_NODE, jive::flt::type>
	symbol_op;
}
}

typedef jive::operation_node<jive::flt::symbol_op>
	jive_fltsymbolicconstant_node;

/**
	\brief Create symbolic constant
	\param graph Graph to create constant in
	\param name Symbol name
	\returns Float value representing constant
	
	Convenience function that either creates a new constant or
	returns the output handle of an existing constant.
*/
jive::output *
jive_fltsymbolicconstant(jive_graph * graph, const char * name);

#endif
