/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_COMPARISON_BITSLESS_H
#define JIVE_TYPES_BITSTRING_COMPARISON_BITSLESS_H

#include <jive/types/bitstring/bitoperation-classes.h>

extern const jive_bitcomparison_operation_class JIVE_BITSLESS_NODE_;
#define JIVE_BITSLESS_NODE (JIVE_BITSLESS_NODE_.base.base)

namespace jive {
namespace bitstring {

class sless_operation final : public jive::bits_compare_operation {
};

}
}

jive_output *
jive_bitsless(struct jive_output * operand1, struct jive_output * operand2);

#endif
