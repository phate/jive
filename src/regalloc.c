/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/regalloc.h>
#include <jive/regalloc/auxnodes.h>
#include <jive/regalloc/color.h>
#include <jive/regalloc/fixup.h>
#include <jive/regalloc/reuse.h>
#include <jive/regalloc/shape.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/stackframe.h>

jive_shaped_graph *
jive_regalloc(struct jive_graph * graph)
{
	jive_shaped_graph * shaped_graph = jive_regalloc_shape(graph);
	jive_regalloc_color(shaped_graph);
	jive_regalloc_fixup(shaped_graph);
	jive_regalloc_auxnodes_replace(shaped_graph);
	jive_subroutine_to_stackframe_map stackframe_map;
	jive_regalloc_stackframe(shaped_graph, stackframe_map);
	jive_regalloc_relocate_stackslots(shaped_graph, stackframe_map);
	jive_regalloc_reuse(shaped_graph);
	
	return shaped_graph;
}
