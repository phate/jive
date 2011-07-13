#ifndef JIVE_VSDG_UNION_H
#define JIVE_VSDG_UNION_H

#include <jive/vsdg/node.h>
#include <jive/vsdg/unionlayout.h>

extern const jive_node_class JIVE_UNIFY_NODE;

typedef struct jive_unify_node jive_unify_node;
typedef struct jive_unify_node_attrs jive_unify_node_attrs;

struct jive_unify_node_attrs {
	jive_node_attrs base;
	jive_union_layout * layout;
};

struct jive_unify_node {
	jive_node base;
	jive_unify_node_attrs attrs;
};

jive_node *
jive_unify_node_create(struct jive_region * region, const jive_union_layout * layout,
	size_t narguments, jive_output * const arguments[]);

jive_output *
jive_unify_create(const jive_union_layout * layout,
	size_t narguments, jive_output * const arguments[]);

static inline jive_unify_node *
jive_unify_node_cast(jive_node * node)
{
	if(node->class_ == &JIVE_UNIFY_NODE) return (jive_unify_node *) node;
	else return 0;
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

static inline jive_choose_node *
jive_choose_node_cast(jive_node * node)
{
	if(node->class_ == &JIVE_CHOOSE_NODE) return (jive_choose_node *) node;
	else return 0;
}

#endif
