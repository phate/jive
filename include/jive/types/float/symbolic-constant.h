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

namespace jive {
namespace flt {

struct symbolicconstant_operation : public unary_operation {
	std::string name;
};

}
}

typedef jive::operation_node<jive::flt::symbolicconstant_operation>
	jive_fltsymbolicconstant_node;

/**
	\brief Create symbolic constant
	\param graph Graph to create constant in
	\param name Symbol name
	\returns Float value representing constant
	
	Convenience function that either creates a new constant or
	returns the output handle of an existing constant.
*/
jive_output *
jive_fltsymbolicconstant(struct jive_graph * graph, const char * name);

#endif
