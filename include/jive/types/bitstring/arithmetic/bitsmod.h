/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_ARITHMETIC_BITSMOD_H
#define JIVE_TYPES_BITSTRING_ARITHMETIC_BITSMOD_H

#include <jive/types/bitstring/bitoperation-classes.h>

extern const jive_bitbinary_operation_class JIVE_BITSMOD_NODE_;
#define JIVE_BITSMOD_NODE (JIVE_BITSMOD_NODE_.base.base)

namespace jive {
namespace bitstring {

class smod_operation final : public jive::bits_binary_operation {
};

}
}

jive_output *
jive_bitsmod(jive_output * operand1, jive_output * operand2);

#endif
