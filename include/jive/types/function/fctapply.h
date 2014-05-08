/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FUNCTION_FCTAPPLY_H
#define JIVE_TYPES_FUNCTION_FCTAPPLY_H

#include <jive/vsdg/node.h>

namespace jive {
namespace fct {

class apply_operation final : public jive::operation {
};

}
}

typedef jive::operation_node<jive::fct::apply_operation> jive_apply_node;

extern const jive_node_class JIVE_APPLY_NODE;

jive_node *
jive_apply_node_create(jive_region * region, jive_output * function,
	size_t narguments, jive_output * const arguments[]);

void
jive_apply_create(jive_output * function, size_t narguments, jive_output * const arguments[],
	jive_output * results[]);

JIVE_EXPORTED_INLINE jive_apply_node *
jive_apply_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_APPLY_NODE))
		return (jive_apply_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const jive_apply_node *
jive_apply_node_const_cast(const struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_APPLY_NODE))
		return (const jive_apply_node *) node;
	else
		return NULL;
}

#endif
