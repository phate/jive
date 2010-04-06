#ifndef JIVE_REGALLOC_H
#define JIVE_REGALLOC_H

#include <jive/types.h>
#include <jive/graph.h>

void
jive_regalloc(jive_graph * graph, const jive_machine * machine, jive_stackframe * frame);

#endif
