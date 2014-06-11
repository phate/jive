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

namespace jive {

class dataitems_operation final : public operation {
};

class datadef_operation final : public operation {
};

class dataobj_operation final : public operation {
};

}

extern const jive_node_class JIVE_DATAITEMS_NODE;
extern const jive_node_class JIVE_DATADEF_NODE;
extern const jive_node_class JIVE_DATAOBJ_NODE;

struct jive_memlayout_mapper;

typedef jive::operation_node<jive::dataitems_operation> jive_dataitems_node;
typedef jive::operation_node<jive::datadef_operation> jive_datadef_node;
typedef jive::operation_node<jive::dataobj_operation> jive_dataobj_node;

jive::output *
jive_dataobj(jive::output * data, struct jive_memlayout_mapper * mapper);

jive::output *
jive_rodataobj(jive::output * data, struct jive_memlayout_mapper * mapper);

jive::output *
jive_bssobj(jive::output * data, struct jive_memlayout_mapper * mapper);

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
