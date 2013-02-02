/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_DATAOBJECT_H
#define JIVE_ARCH_DATAOBJECT_H

#include <jive/common.h>

#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>

extern const jive_node_class JIVE_DATAITEMS_NODE;
extern const jive_node_class JIVE_DATADEF_NODE;
extern const jive_node_class JIVE_DATAOBJ_NODE;

struct jive_memlayout_mapper;

typedef struct jive_node jive_dataitems_node;
typedef struct jive_node jive_datadef_node;
typedef struct jive_node jive_dataobj_node;

jive_output *
jive_dataobj(jive_output * data, struct jive_memlayout_mapper * mapper);

jive_output *
jive_rodataobj(jive_output * data, struct jive_memlayout_mapper * mapper);

jive_output *
jive_bssobj(jive_output * data, struct jive_memlayout_mapper * mapper);

JIVE_EXPORTED_INLINE jive_dataitems_node *
jive_dataitems_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_DATAITEMS_NODE))
		return (jive_dataitems_node *) node;
	else
		return 0;
}

JIVE_EXPORTED_INLINE jive_datadef_node *
jive_datadef_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_DATADEF_NODE))
		return (jive_datadef_node *) node;
	else
		return 0;
}

JIVE_EXPORTED_INLINE jive_dataobj_node *
jive_dataobj_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_DATAOBJ_NODE))
		return (jive_dataobj_node *) node;
	else
		return 0;
}

#endif
