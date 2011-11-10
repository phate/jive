#ifndef JIVE_VSDG_RECORD_H
#define JIVE_VSDG_RECORD_H

#include <jive/common.h>

#include <jive/vsdg/node.h>
#include <jive/vsdg/basetype.h>
#include <jive/vsdg/recordtype.h>

extern const jive_node_class JIVE_GROUP_NODE;

typedef struct jive_group_node jive_group_node;
typedef struct jive_group_node_attrs jive_group_node_attrs;

struct jive_group_node_attrs {
	jive_node_attrs base;
	const jive_record_declaration * decl;
};

struct jive_group_node {
	jive_node base;
	jive_group_node_attrs attrs;
};

jive_node *
jive_group_node_create(struct jive_region * region, const jive_record_declaration * decl,
	size_t narguments, jive_output * const arguments[]);

jive_output *
jive_group_create(const jive_record_declaration * decl,
	size_t narguments, jive_output * arguments[const]);

JIVE_EXPORTED_INLINE jive_group_node *
jive_group_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_GROUP_NODE)
		return (jive_group_node *) node;
	else
		return 0;
}

extern const jive_node_class JIVE_SELECT_NODE;

typedef struct jive_select_node jive_select_node;
typedef struct jive_select_node_attrs jive_select_node_attrs;

struct jive_select_node_attrs {
	jive_node_attrs base;
	size_t element;
};

struct jive_select_node {
	jive_node base;
	jive_select_node_attrs attrs;
};

jive_select_node *
jive_select_node_create(struct jive_region * region, size_t element, jive_output * operand);

jive_output *
jive_select_create(size_t element, jive_output * operand);

JIVE_EXPORTED_INLINE jive_select_node *
jive_select_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_SELECT_NODE)
		return (jive_select_node *) node;
	else
		return 0;
}

#endif
