#ifndef JIVE_REGALLOC_SHAPE_H
#define JIVE_REGALLOC_SHAPE_H

#include <jive/types.h>
#include <jive/graph.h>
#include <jive/machine.h>
#include <jive/instruction.h>

void
jive_regalloc_shape(jive_graph * graph, const jive_machine * machine,
	jive_stackframe * stack, jive_instruction_sequence * seq);

#endif
