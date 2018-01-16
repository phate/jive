/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_BACKEND_I386_INSTRUCTIONMATCH_H
#define JIVE_BACKEND_I386_INSTRUCTIONMATCH_H

#include <jive/arch/regselector.h>

namespace jive {
namespace i386 {

void
match_instructions(jive::graph * graph, const jive::register_selector * regselector);

}}

#endif
