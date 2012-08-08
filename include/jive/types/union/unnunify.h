#ifndef JIVE_TYPES_UNION_UNNUNIFY_H
#define JIVE_TYPES_UNION_UNNUNIFY_H

#include <jive/vsdg/node.h>

struct jive_union_declaration;

extern const jive_node_class JIVE_UNIFY_NODE;

typedef struct jive_unify_node jive_unify_node;
typedef struct jive_unify_node_attrs jive_unify_node_attrs;

struct jive_unify_node_attrs {
	jive_node_attrs base;
	const struct jive_union_declaration * decl;
	size_t option;
};

struct jive_unify_node {
	jive_node base;
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

#endif
