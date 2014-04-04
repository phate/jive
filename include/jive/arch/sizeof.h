/*
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_SIZEOF_H
#define JIVE_ARCH_SIZEOF_H

#include <jive/vsdg/node.h>

struct jive_memlayout_mapper;
struct jive_value_type;

extern const jive_node_class JIVE_SIZEOF_NODE;

typedef struct jive_sizeof_node jive_sizeof_node;
typedef struct jive_sizeof_node_attrs jive_sizeof_node_attrs;

struct jive_sizeof_node_attrs : public jive_node_attrs {
	struct jive_value_type * type;
};

struct jive_sizeof_node {
	jive_node base;
	jive_sizeof_node_attrs attrs;
};

struct jive_node *
jive_sizeof_node_create(struct jive_region * region,
	const struct jive_value_type * type);

struct jive_output *
jive_sizeof_create(struct jive_region * region,
	const struct jive_value_type * type);

JIVE_EXPORTED_INLINE jive_sizeof_node *
jive_sizeof_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_SIZEOF_NODE))
		return (jive_sizeof_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_value_type *
jive_sizeof_node_get_type(const struct jive_sizeof_node * node)
{
	return node->attrs.type;
}

void
jive_sizeof_node_reduce(const jive_sizeof_node * node, struct jive_memlayout_mapper * mapper);

#endif
