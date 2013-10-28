/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_REAL_RLCONSTANT_H
#define JIVE_TYPES_REAL_RLCONSTANT_H

#include <jive/util/bitstring.h>
#include <jive/vsdg/node.h>

extern const jive_node_class JIVE_RLCONSTANT_NODE;

typedef struct jive_rlconstant_node jive_rlconstant_node;
typedef struct jive_rlconstant_node_attrs jive_rlconstant_node_attrs;

struct jive_rlconstant_node_attrs {
	jive_node_attrs base;
	bool sign;
	size_t nnbits;
	char * numerator;
	size_t ndbits;
	char * denominator;
};

struct jive_rlconstant_node {
	jive_node base;
	jive_rlconstant_node_attrs attrs;
};

struct jive_output *
jive_rlconstant(struct jive_graph * graph, bool sign, size_t nnbits, const char * numerator,
	size_t ndbits, const char * denominator);

JIVE_EXPORTED_INLINE struct jive_output *
jive_rlconstant_unsigned(struct jive_graph * graph, uint64_t numerator, uint64_t denominator)
{
	char n[64], d[64];
	jive_bitstring_init_unsigned(n, 64, numerator);
	jive_bitstring_init_unsigned(d, 64, denominator);

	return jive_rlconstant(graph, false, 64, n, 64, d);
}

JIVE_EXPORTED_INLINE struct jive_output *
jive_rlconstant_signed(struct jive_graph * graph, int64_t numerator, int64_t denominator)
{
	char n[64], d[64];
	size_t nsign = (numerator >> 63) & 1;
	size_t dsign = (denominator >> 63) & 1;
	jive_bitstring_init_unsigned(n, 64, nsign ? -numerator : numerator);
	jive_bitstring_init_unsigned(d, 64, dsign ? -denominator : denominator);

	return jive_rlconstant(graph, nsign ^ dsign, 64, n, 64, d);
}

struct jive_output *
jive_rlconstant_float(struct jive_graph * graph, float f);

struct jive_output *
jive_rlconstant_double(struct jive_graph * graph, double f);

JIVE_EXPORTED_INLINE struct jive_rlconstant_node *
jive_rlconstant_node_cast(struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_RLCONSTANT_NODE))
		return (struct jive_rlconstant_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_rlconstant_node *
jive_rlconstant_node_const_cast(const struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_RLCONSTANT_NODE))
		return (const struct jive_rlconstant_node *) node;
	else
		return NULL;
}

#endif
