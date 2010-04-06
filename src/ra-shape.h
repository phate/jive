#ifndef JIVE_INTERNAL_RA_SHAPE_H
#define JIVE_INTERNAL_RA_SHAPE_H

#include <jive/types.h>
#include <jive/graph.h>

jive_graphcut *
jive_graphshape(jive_graph * graph, const jive_machine * machine,
	jive_stackframe * stackframe);

#endif
