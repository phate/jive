/*
 * Copyright 2013 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_SUBROUTINE_NODES_H
#define JIVE_ARCH_SUBROUTINE_NODES_H

#include <jive/vsdg/node.h>

extern const jive_node_class JIVE_SUBROUTINE_ENTER_NODE;
extern const jive_node_class JIVE_SUBROUTINE_LEAVE_NODE;
extern const jive_node_class JIVE_SUBROUTINE_NODE;

typedef struct jive_subroutine_node_attrs jive_subroutine_node_attrs;

typedef struct jive_subroutine_enter_node jive_subroutine_enter_node;
typedef struct jive_subroutine_leave_node jive_subroutine_leave_node;
typedef struct jive_subroutine_node jive_subroutine_node;

struct jive_subroutine_node_attrs {
	jive_node_attrs base;
	struct jive_subroutine * subroutine;
};

struct jive_subroutine_node {
	jive_node base;
	struct jive_subroutine_node_attrs attrs;
};

struct jive_subroutine_enter_node {
	jive_node base;
};

struct jive_subroutine_leave_node {
	jive_node base;
};

JIVE_EXPORTED_INLINE jive_subroutine_node *
jive_subroutine_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_SUBROUTINE_NODE))
		return (jive_subroutine_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_subroutine_enter_node *
jive_subroutine_enter_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_SUBROUTINE_ENTER_NODE))
		return (jive_subroutine_enter_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_subroutine_leave_node *
jive_subroutine_leave_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_SUBROUTINE_LEAVE_NODE))
		return (jive_subroutine_leave_node *) node;
	else
		return NULL;
}

jive_node *
jive_subroutine_enter_node_create(jive_region * region);

jive_node *
jive_subroutine_leave_node_create(jive_region * region, jive_output * control_transfer);

jive_node *
jive_subroutine_node_create(jive_region * subroutine_region, struct jive_subroutine * subroutine);

#endif
