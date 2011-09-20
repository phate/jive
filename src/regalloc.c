#include <jive/regalloc.h>

#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/shape.h>
#include <jive/regalloc/color.h>
#include <jive/regalloc/fixup.h>
#include <jive/regalloc/auxnodes.h>
#include <jive/regalloc/reuse.h>
#include <jive/regalloc/stackframe.h>

jive_shaped_graph *
jive_regalloc(struct jive_graph * graph, const struct jive_transfer_instructions_factory * xfer)
{
	jive_shaped_graph * shaped_graph = jive_regalloc_shape(graph);
	jive_regalloc_color(shaped_graph);
	jive_regalloc_fixup(shaped_graph);
	jive_regalloc_auxnodes_replace(shaped_graph, xfer);
	jive_regalloc_stackframe(shaped_graph);
	jive_regalloc_relocate_stackslots(shaped_graph);
	jive_regalloc_reuse(shaped_graph);
	
	return shaped_graph;
}
