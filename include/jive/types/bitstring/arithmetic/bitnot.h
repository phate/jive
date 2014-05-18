/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_ARITHMETIC_BITNOT_H
#define JIVE_TYPES_BITSTRING_ARITHMETIC_BITNOT_H

#include <jive/types/bitstring/bitoperation-classes.h>

extern const jive_bitunary_operation_class JIVE_BITNOT_NODE_;
#define JIVE_BITNOT_NODE (JIVE_BITNOT_NODE_.base.base)

namespace jive {
namespace bitstring {

class not_operation final : public jive::bits_unary_operation {
};

}
}

/**
	\brief Create bitnot
	\param operand Input value
	\returns Bitstring value representing not
	
	Convenience function to create negation of value.
*/
jive_output *
jive_bitnot(jive_output * operand);

#endif
