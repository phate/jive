/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_BACKEND_I386_CALL_H
#define JIVE_BACKEND_I386_CALL_H

struct jive_node;
struct jive_call_node;

struct jive_node *
jive_i386_call_node_substitute(struct jive_call_node * node);

#endif
