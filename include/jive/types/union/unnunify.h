/*
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_UNION_UNNUNIFY_H
#define JIVE_TYPES_UNION_UNNUNIFY_H

#include <jive/vsdg/operators/unary.h>

struct jive_union_declaration;

/* unify node */

extern const jive_unary_operation_class JIVE_UNIFY_NODE_;
#define JIVE_UNIFY_NODE (JIVE_UNIFY_NODE_.base)

typedef struct jive_unify_node jive_unify_node;
typedef struct jive_unify_node_attrs jive_unify_node_attrs;

struct jive_unify_node_attrs : public jive_node_attrs {
	const struct jive_union_declaration * decl;
	size_t option;
};

struct jive_unify_node : public jive_node {
	jive_unify_node_attrs attrs;
};

jive_node *
jive_unify_node_create(struct jive_region * region, const struct jive_union_declaration * decl,
	size_t option, jive_output * const operand);

jive_output *
jive_unify_create(const struct jive_union_declaration * decl,
	size_t option, jive_output * const operand);

JIVE_EXPORTED_INLINE jive_unify_node *
jive_unify_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_UNIFY_NODE))
		return (jive_unify_node *) node;
	else
		return NULL;
}

/* empty unify node */

extern const jive_node_class JIVE_EMPTY_UNIFY_NODE;

typedef struct jive_empty_unify_node jive_empty_unify_node;
typedef struct jive_empty_unify_node_attrs jive_empty_unify_node_attrs;

struct jive_empty_unify_node_attrs : public jive_node_attrs {
	const struct jive_union_declaration * decl;
};

struct jive_empty_unify_node : public jive_node {
	jive_empty_unify_node_attrs attrs;
};

jive_output *
jive_empty_unify_create(struct jive_graph * graph, const struct jive_union_declaration * decl);

JIVE_EXPORTED_INLINE jive_empty_unify_node *
jive_empty_unify_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_EMPTY_UNIFY_NODE))
		return (jive_empty_unify_node *) node;
	else
		return NULL;
}

#endif
