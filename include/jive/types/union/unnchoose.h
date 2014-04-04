/*
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_UNION_UNNCHOOSE_H
#define JIVE_TYPES_UNION_UNNCHOOSE_H

#include <jive/vsdg/operators/unary.h>

extern const jive_unary_operation_class JIVE_CHOOSE_NODE_;
#define JIVE_CHOOSE_NODE (JIVE_CHOOSE_NODE_.base)

typedef struct jive_choose_node jive_choose_node;
typedef struct jive_choose_node_attrs jive_choose_node_attrs;

struct jive_choose_node_attrs : public jive_node_attrs {
	size_t element;
};

struct jive_choose_node : public jive_node {
	jive_choose_node_attrs attrs;
};

jive_output *
jive_choose_node_create(struct jive_region * region, size_t element, jive_output * operand);

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
