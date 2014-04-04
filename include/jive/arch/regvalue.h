/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_REGVALUE_H
#define JIVE_ARCH_REGVALUE_H

#include <stdint.h>

#include <jive/arch/registers.h>
#include <jive/vsdg/node.h>

extern const jive_node_class JIVE_REGVALUE_NODE;

typedef struct jive_regvalue_node jive_regvalue_node;
typedef struct jive_regvalue_node_attrs jive_regvalue_node_attrs;

struct jive_regvalue_node_attrs : public jive_node_attrs {
	const jive_register_class * regcls;
};

struct jive_regvalue_node {
	jive_node base;
	jive_regvalue_node_attrs attrs;
};

/**
	\brief Create register constant
	\param ctl Control of region entry where value is alive
	\param regcls Register class
	\param value Value to be represented
	\returns Bitstring value representing constant, constrained to register class
	
	Convenience function that either creates a new constant or
	returns the output handle of an existing constant.
*/
jive_output *
jive_regvalue(jive_output * ctl, const jive_register_class * regcls, jive_output * value);

JIVE_EXPORTED_INLINE jive_regvalue_node *
jive_regvalue_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_REGVALUE_NODE))
		return (jive_regvalue_node *) node;
	else
		return 0;
}

#endif
