/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_UNION_UNNCHOOSE_H
#define JIVE_TYPES_UNION_UNNCHOOSE_H

#include <jive/vsdg/operators/unary.h>

extern const jive_unary_operation_class JIVE_CHOOSE_NODE_;
#define JIVE_CHOOSE_NODE (JIVE_CHOOSE_NODE_.base)

namespace jive {
namespace unn {

class choose_operation final : public unary_operation {
public:
	inline constexpr
	choose_operation(size_t element) noexcept : element_(element) {}

	inline size_t
	element() const noexcept { return element_; }

private:
	size_t element_;
};

}
}

typedef jive::operation_node<jive::unn::choose_operation> jive_choose_node;

jive_output *
jive_choose_create(size_t element, jive_output * operand);

JIVE_EXPORTED_INLINE jive_choose_node *
jive_choose_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_CHOOSE_NODE))
		return (jive_choose_node *) node;
	else
		return NULL;
}

#endif
