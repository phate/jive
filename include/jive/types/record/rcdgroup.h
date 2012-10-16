/*
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_RECORD_RCDGROUP_H
#define JIVE_TYPES_RECORD_RCDGROUP_H

#include <jive/vsdg/node.h>
#include <jive/types/record/rcdtype.h>

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

jive_output *
jive_empty_group_create(struct jive_graph * graph, const jive_record_declaration * decl);

JIVE_EXPORTED_INLINE jive_group_node *
jive_group_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_GROUP_NODE)
		return (jive_group_node *) node;
	else
		return 0;
}


#endif
