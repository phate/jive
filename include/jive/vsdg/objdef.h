#ifndef JIVE_VSDG_OBJDEF_H
#define JIVE_VSDG_OBJDEF_H

#include <jive/vsdg/node.h>

struct jive_label;
struct jive_output;

typedef struct jive_objdef_node jive_objdef_node;
typedef struct jive_objdef_node_attrs jive_objdef_node_attrs;

extern const struct jive_node_class JIVE_OBJDEF_NODE;

struct jive_objdef_node_attrs {
	jive_node_attrs base;
	char * name;
	struct jive_label * start;
	struct jive_label * end;
};

struct jive_objdef_node {
	jive_node base;
	jive_objdef_node_attrs attrs;
};

struct jive_node *
jive_objdef_node_create(struct jive_output * output, const char * name);

JIVE_EXPORTED_INLINE jive_objdef_node *
jive_objdef_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_OBJDEF_NODE)
		return (jive_objdef_node *) node;
	else
		return NULL;
}

#endif
