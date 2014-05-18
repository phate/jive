/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_ARITHMETIC_BITUMOD_H
#define JIVE_TYPES_BITSTRING_ARITHMETIC_BITUMOD_H

#include <jive/types/bitstring/bitoperation-classes.h>

extern const jive_bitbinary_operation_class JIVE_BITUMOD_NODE_;
#define JIVE_BITUMOD_NODE (JIVE_BITUMOD_NODE_.base.base)

namespace jive {
namespace bitstring {

class umod_operation final : public jive::bits_binary_operation {
};

}
}

jive_output *
jive_bitumod(jive_output * operand1, jive_output * operand2);

#endif
