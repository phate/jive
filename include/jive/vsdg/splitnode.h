/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_SPLITNODE_H
#define JIVE_VSDG_SPLITNODE_H

/* auxiliary node to represent a "split" of the same value */

#include <jive/common.h>

#include <jive/vsdg/node.h>

struct jive_resource_class;
struct jive_shaped_graph;

typedef struct jive_splitnode_attrs jive_splitnode_attrs;

struct jive_splitnode_attrs : public jive_node_attrs {
	const struct jive_resource_class * in_class;
	const struct jive_resource_class * out_class;
};

struct jive_splitnode {
	jive_node base;
	jive_splitnode_attrs attrs;
};

typedef struct jive_splitnode jive_splitnode;
extern const jive_node_class JIVE_SPLITNODE;

jive_node *
jive_splitnode_create(struct jive_region * region,
	const jive_type * in_type,
	struct jive_output * in_origin,
	const struct jive_resource_class * in_class,
	const jive_type * out_type,
	const struct jive_resource_class * out_class);

JIVE_EXPORTED_INLINE jive_splitnode *
jive_splitnode_cast(jive_node * self)
{
	if (self->class_ == &JIVE_SPLITNODE)
		return (jive_splitnode *) self;
	else
		return 0;
}

#endif
