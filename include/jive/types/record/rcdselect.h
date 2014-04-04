/*
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_RECORD_RCDSELECT_H
#define JIVE_TYPES_RECORD_RCDSELECT_H

#include <jive/vsdg/operators/unary.h>

extern const jive_unary_operation_class JIVE_SELECT_NODE_;
#define JIVE_SELECT_NODE (JIVE_SELECT_NODE_.base)

typedef struct jive_select_node jive_select_node;
typedef struct jive_select_node_attrs jive_select_node_attrs;

struct jive_select_node_attrs : public jive_node_attrs {
	size_t element;
};

struct jive_select_node : public jive_node {
	jive_select_node_attrs attrs;
};

jive_output *
jive_select_node_create(struct jive_region * region, size_t element, jive_output * operand);

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
