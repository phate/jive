#ifndef JIVE_VSDG_OBJDEF_H
#define JIVE_VSDG_OBJDEF_H

struct jive_node;
struct jive_node_class;
struct jive_output;

extern const struct jive_node_class JIVE_OBJDEF_NODE;

struct jive_node *
jive_objdef_node_create(struct jive_output * output, const char * name);

#endif
