#ifndef JIVE_TYPES_UNION_UNION_H
#define JIVE_TYPES_UNION_UNION_H

#include <jive/common.h>

#include <jive/vsdg/node.h>
#include <jive/types/union/unntype.h>

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
	if (node->class_ == &JIVE_UNIFY_NODE)
		return (jive_unify_node *) node;
	else
		return 0;
}

extern const jive_node_class JIVE_CHOOSE_NODE;

typedef struct jive_choose_node jive_choose_node;
typedef struct jive_choose_node_attrs jive_choose_node_attrs;

struct jive_choose_node_attrs {
	jive_node_attrs base;
	size_t element;
};

struct jive_choose_node {
	jive_node base;
	jive_choose_node_attrs attrs;
};

jive_choose_node *
jive_choose_node_create(struct jive_region * region, size_t element, jive_output * operand);

jive_output *
jive_choose_create(size_t element, jive_output * operand);

JIVE_EXPORTED_INLINE jive_choose_node *
jive_choose_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_CHOOSE_NODE)
		return (jive_choose_node *) node;
	else
		return 0;
}

#endif
