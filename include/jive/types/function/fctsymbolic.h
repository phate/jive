/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FUNCTION_FCTSYMBOLIC_H
#define JIVE_TYPES_FUNCTION_FCTSYMBOLIC_H

#include <jive/vsdg/node.h>
#include <jive/types/function/fcttype.h>

extern const jive_node_class JIVE_SYMBOLICFUNCTION_NODE;

typedef struct jive_symbolicfunction_node jive_symbolicfunction_node;
typedef struct jive_symbolicfunction_node_attrs jive_symbolicfunction_node_attrs;

struct jive_symbolicfunction_node_attrs : public jive_node_attrs {
	const char * name;
	jive_function_type type;
};

struct jive_symbolicfunction_node {
	jive_node base;
	jive_symbolicfunction_node_attrs attrs; 
};

jive_node *
jive_symbolicfunction_node_create(struct jive_graph * graph, const char * name, const jive_function_type * type);

jive_output *
jive_symbolicfunction_create(struct jive_graph * graph, const char * name, const jive_function_type * type);

JIVE_EXPORTED_INLINE jive_symbolicfunction_node *
jive_symbolicfunction_node_cast(jive_node * node)
{
	if(node->class_ == &JIVE_SYMBOLICFUNCTION_NODE) return (jive_symbolicfunction_node *) node;
	else return 0;
}

#endif
