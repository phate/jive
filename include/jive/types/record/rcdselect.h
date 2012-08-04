#ifndef JIVE_TYPES_RECORD_RCDSELECT_H
#define JIVE_TYPES_RECORD_RCDSELECT_H

#include <jive/vsdg/node.h>
#include <jive/types/record/rcdtype.h>

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
