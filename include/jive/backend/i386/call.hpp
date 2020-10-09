/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_BACKEND_I386_CALL_HPP
#define JIVE_BACKEND_I386_CALL_HPP

namespace jive {

class node;

namespace i386 {

jive::node *
substitute_call(jive::node * node);

}}

#endif
