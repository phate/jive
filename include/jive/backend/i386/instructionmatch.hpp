/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_BACKEND_I386_INSTRUCTIONMATCH_HPP
#define JIVE_BACKEND_I386_INSTRUCTIONMATCH_HPP

#include <jive/arch/regselector.hpp>

namespace jive {
namespace i386 {

void
match_instructions(jive::graph * graph);

}}

#endif
