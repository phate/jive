/*
 * Copyright 2010 2011 2012 2013 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_OBJDEF_H
#define JIVE_VSDG_OBJDEF_H

#include <jive/vsdg/node.h>

struct jive_label;
struct jive_linker_symbol;
struct jive_output;

typedef struct jive_objdef_node jive_objdef_node;
typedef struct jive_objdef_node_attrs jive_objdef_node_attrs;

extern const struct jive_node_class JIVE_OBJDEF_NODE;

struct jive_objdef_node_attrs : public jive_node_attrs {
	char * name;
	const struct jive_linker_symbol * symbol;
};

struct jive_objdef_node {
	jive_node base;
	jive_objdef_node_attrs attrs;
};

struct jive_node *
jive_objdef_node_create(
	struct jive_output * output,
	const char * name,
	const struct jive_linker_symbol * symbol);

JIVE_EXPORTED_INLINE jive_objdef_node *
jive_objdef_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_OBJDEF_NODE)
		return (jive_objdef_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const char *
jive_objdef_node_get_name(const jive_objdef_node * node)
{
	return node->attrs.name;
}

#endif
