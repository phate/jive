#include <jive/regalloc.h>
#include <jive/regalloc/shape.h>
#include <jive/regalloc/color.h>
#include <jive/regalloc/fixup.h>
#include <jive/regalloc/auxnodes.h>
#include <jive/regalloc/regreuse.h>

void
jive_regalloc(struct jive_graph * graph, const struct jive_transfer_instructions_factory * xfer)
{
	jive_regalloc_shape(graph);
	jive_regalloc_color(graph);
	jive_regalloc_fixup(graph);
	jive_regalloc_auxnodes_replace(graph, xfer);
	jive_regalloc_regreuse(graph);
}

