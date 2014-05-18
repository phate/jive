/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_COMPARISON_BITSGREATEREQ_H
#define JIVE_TYPES_BITSTRING_COMPARISON_BITSGREATEREQ_H

#include <jive/types/bitstring/bitoperation-classes.h>

extern const jive_bitcomparison_operation_class JIVE_BITSGREATEREQ_NODE_;
#define JIVE_BITSGREATEREQ_NODE (JIVE_BITSGREATEREQ_NODE_.base.base)

namespace jive {
namespace bitstring {

class sgreatereq_operation final : public jive::bits_compare_operation {
};

}
}

jive_output *
jive_bitsgreatereq(struct jive_output * operand1, struct jive_output * operand2);

#endif
