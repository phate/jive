/*
 * Copyright 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FUNCTION_FCTAPPLY_H
#define JIVE_TYPES_FUNCTION_FCTAPPLY_H

#include <jive/vsdg/node.h>

typedef struct jive_node jive_apply_node;

extern const jive_node_class JIVE_APPLY_NODE;

jive_node *
jive_apply_node_create(jive_region * region, jive_output * function,
	size_t narguments, jive_output * const arguments[]);

JIVE_EXPORTED_INLINE jive_apply_node *
jive_apply_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_APPLY_NODE))
		return (jive_apply_node *) node;
	else
		return NULL;
}

#endif
