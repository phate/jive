/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_DOUBLE_DBLCONSTANT_H
#define JIVE_TYPES_DOUBLE_DBLCONSTANT_H

#include <jive/vsdg/node.h>

extern const jive_node_class JIVE_DBLCONSTANT_NODE;

typedef struct jive_dblconstant_node jive_dblconstant_node;
typedef struct jive_dblconstant_node_attrs jive_dblconstant_node_attrs;

struct jive_dblconstant_node_attrs {
	jive_node_attrs base;
	uint64_t value;
};

struct jive_dblconstant_node {
	jive_node base;
	jive_dblconstant_node_attrs attrs;
};

struct jive_output *
jive_dblconstant(struct jive_graph * graph, uint64_t value);

JIVE_EXPORTED_INLINE struct jive_output *
jive_dblconstant_double(struct jive_graph * graph, double value)
{
	union u {
		uint64_t i;
		double f;
	};

	union u c;
	c.f = value;

	return jive_dblconstant(graph, c.i);
}

JIVE_EXPORTED_INLINE struct jive_dblconstant_node *
jive_dblconstant_node_cast(struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_DBLCONSTANT_NODE))
		return (jive_dblconstant_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_dblconstant_node *
jive_dblconstant_node_const_cast(const struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_DBLCONSTANT_NODE))
		return (const jive_dblconstant_node *) node;
	else
		return NULL;
}

#endif
