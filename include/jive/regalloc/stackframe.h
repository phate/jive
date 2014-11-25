/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_REGALLOC_STACKFRAME_H
#define JIVE_REGALLOC_STACKFRAME_H

#include <unordered_map>

#include <jive/arch/subroutine.h>

struct jive_shaped_graph;

typedef std::unordered_map<jive_node *, jive_subroutine_stackframe_info>
	jive_subroutine_to_stackframe_map;

void
jive_regalloc_stackframe(
	jive_shaped_graph * shaped_graph,
	jive_subroutine_to_stackframe_map & stackframe_map);

void
jive_regalloc_relocate_stackslots(
	jive_shaped_graph * shaped_graph,
	jive_subroutine_to_stackframe_map & stackframe_map);

#endif
