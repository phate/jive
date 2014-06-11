/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_COMPARISON_BITSGREATER_H
#define JIVE_TYPES_BITSTRING_COMPARISON_BITSGREATER_H

#include <jive/types/bitstring/bitoperation-classes.h>

extern const jive_bitcomparison_operation_class JIVE_BITSGREATER_NODE_;
#define JIVE_BITSGREATER_NODE (JIVE_BITSGREATER_NODE_.base.base)

namespace jive {
namespace bitstring {

class sgreater_operation final : public jive::bits_compare_operation {
};

}
}


jive::output *
jive_bitsgreater(jive::output * operand1, jive::output * operand2);

#endif
