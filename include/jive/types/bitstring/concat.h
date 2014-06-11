/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_CONCAT_H
#define JIVE_TYPES_BITSTRING_CONCAT_H

#include <jive/types/bitstring/type.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/operators.h>

extern const jive_binary_operation_class JIVE_BITCONCAT_NODE_;
#define JIVE_BITCONCAT_NODE (JIVE_BITCONCAT_NODE_.base)

namespace jive {
namespace bitstring {

class concat_operation final : public jive::operation {
};

}
}

jive::output *
jive_bitconcat(size_t noperands, jive::output * const * operands);

#endif
