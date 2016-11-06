/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_NODE_PRIVATE_H
#define JIVE_VSDG_NODE_PRIVATE_H

#include <jive/util/list.h>
#include <jive/vsdg/basetype.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/tracker.h>

/* inheritable node member functions */

jive_node *
jive_opnode_create(
	const jive::operation & op,
	jive_region * region,
	const std::vector<jive::oport*> & operands);

#endif
