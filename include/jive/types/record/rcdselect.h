/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_RECORD_RCDSELECT_H
#define JIVE_TYPES_RECORD_RCDSELECT_H

#include <jive/vsdg/operators/unary.h>

namespace jive {
namespace rcd {

class select_operation final : public jive::unary_operation {
public:
	size_t element;
};

}
}

extern const jive_unary_operation_class JIVE_SELECT_NODE_;
#define JIVE_SELECT_NODE (JIVE_SELECT_NODE_.base)

typedef jive::operation_node<jive::rcd::select_operation> jive_select_node;

jive_output *
jive_select_create(size_t element, jive_output * operand);

JIVE_EXPORTED_INLINE jive_select_node *
jive_select_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_SELECT_NODE))
		return (jive_select_node *) node;
	else
		return 0;
}

#endif
