/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_INTEGRAL_ITGCONSTANT_H
#define JIVE_TYPES_INTEGRAL_ITGCONSTANT_H

#include <jive/vsdg/operators/nullary.h>

typedef struct jive_itgconstant_node jive_itgconstant_node;
typedef struct jive_itgconstant_node_attrs jive_itgconstant_node_attrs;

extern const jive_nullary_operation_class JIVE_ITGCONSTANT_NODE;

struct jive_itgconstant_node_attrs {
	jive_node_attrs base;
	size_t nbits;
	char * bits;
};

struct jive_itgconstant_node {
	jive_node base;
	jive_itgconstant_node_attrs attrs;
};

struct jive_output *
jive_itgconstant(struct jive_graph * graph, size_t nbits, const char bits[]);

JIVE_EXPORTED_INLINE struct jive_itgconstant_node *
jive_itgconstant_node_cast(struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_ITGCONSTANT_NODE))
		return (jive_itgconstant_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_itgconstant_node *
jive_itgconstant_node_const_cast(const struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_ITGCONSTANT_NODE))
		return (const jive_itgconstant_node *) node;
	else
		return NULL;
}

#endif
