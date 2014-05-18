/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_ARITHMETIC_BITASHR_H
#define JIVE_TYPES_BITSTRING_ARITHMETIC_BITASHR_H

#include <jive/types/bitstring/bitoperation-classes.h>

extern const jive_bitbinary_operation_class JIVE_BITASHR_NODE_;
#define JIVE_BITASHR_NODE (JIVE_BITASHR_NODE_.base.base)

namespace jive {
namespace bitstring {

class ashr_operation final : public jive::bits_binary_operation {
};

}
}

jive_output *
jive_bitashr(jive_output * operand, jive_output * shift);

#endif
