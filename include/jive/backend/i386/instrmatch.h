/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_BACKEND_I386_INSTRMATCH_H
#define JIVE_BACKEND_I386_INSTRMATCH_H

#include <jive/arch/regselector.h>

void
jive_i386_match_instructions(jive::graph * graph, const jive::register_selector * regselector);

#endif
