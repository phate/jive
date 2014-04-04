/*
 * Copyright 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_FLTCONSTANT_H
#define JIVE_TYPES_FLOAT_FLTCONSTANT_H

#include <jive/vsdg/node.h>

extern const jive_node_class JIVE_FLTCONSTANT_NODE;

typedef struct jive_fltconstant_node jive_fltconstant_node;
typedef struct jive_fltconstant_node_attrs jive_fltconstant_node_attrs;

struct jive_fltconstant_node_attrs : public jive_node_attrs {
	uint32_t value;
};

struct jive_fltconstant_node {
	jive_node base;
	jive_fltconstant_node_attrs attrs;
};

jive_output *
jive_fltconstant(struct jive_graph * graph, uint32_t value);

JIVE_EXPORTED_INLINE jive_output *
jive_fltconstant_float(struct jive_graph * graph, float value)
{
	union u {
		uint32_t i;
		float f;
	};

	union u c;
	c.f = value;

	return jive_fltconstant(graph, c.i);
}

JIVE_EXPORTED_INLINE jive_fltconstant_node *
jive_fltconstant_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_FLTCONSTANT_NODE))
		return (jive_fltconstant_node *) node;
	else
		return NULL;
}

#endif
