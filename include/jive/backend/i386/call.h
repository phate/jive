/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_BACKEND_I386_CALL_H
#define JIVE_BACKEND_I386_CALL_H

#include <jive/arch/call.h>

jive::node *
jive_i386_call_node_substitute(
	jive::node * node,
	const jive::call_op & op);

#endif
