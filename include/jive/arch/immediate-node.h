/*
 * Copyright 2013 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_IMMEDIATE_NODE_H
#define JIVE_ARCH_IMMEDIATE_NODE_H

#include <string.h>

#include <jive/context.h>
#include <jive/arch/immediate-value.h>
#include <jive/arch/linker-symbol.h>
#include <jive/vsdg/node.h>

typedef struct jive_immediate_node jive_immediate_node;
typedef struct jive_immediate_node_attrs jive_immediate_node_attrs;

extern const jive_node_class JIVE_IMMEDIATE_NODE;

struct jive_immediate_node_attrs {
	jive_node_attrs base;
	jive_immediate value;
};

struct jive_immediate_node {
	jive_node base;
	jive_immediate_node_attrs attrs;
};

jive_output *
jive_immediate_create(
	struct jive_graph * graph,
	const jive_immediate * immediate_value);

JIVE_EXPORTED_INLINE jive_immediate_node *
jive_immediate_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_IMMEDIATE_NODE))
		return (jive_immediate_node *) node;
	else
		return 0;
}

#endif
