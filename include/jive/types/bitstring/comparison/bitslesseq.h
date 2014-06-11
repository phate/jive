/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_COMPARISON_BITSLESSEQ_H
#define JIVE_TYPES_BITSTRING_COMPARISON_BITSLESSEQ_H

#include <jive/types/bitstring/bitoperation-classes.h>

extern const jive_bitcomparison_operation_class JIVE_BITSLESSEQ_NODE_;
#define JIVE_BITSLESSEQ_NODE (JIVE_BITSLESSEQ_NODE_.base.base)

namespace jive {
namespace bitstring {

class slesseq_operation final : public jive::bits_compare_operation {
};

}
}

jive::output *
jive_bitslesseq(jive::output * operand1, jive::output * operand2);

#endif
