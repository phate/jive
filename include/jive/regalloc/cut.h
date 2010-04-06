#ifndef JIVE_REGALLOC_CUT_H
#define JIVE_REGALLOC_CUT_H

#include <jive/regalloc/util.h>

/**
	\brief Horizontal cut through the graph
*/
struct jive_graphcut {
	jive_graph * graph;
	
	/* pointers to upper/lower layer */
	jive_graphcut * upper;
	jive_graphcut * lower;
	
	jive_instruction * first, * last;
};

jive_graphcut *
jive_graphcut_create(jive_graph * graph, jive_graphcut * lower);

void
jive_regalloc_add_instruction_between(jive_instruction * instr,
	jive_instruction * prev, jive_instruction * next,
	const jive_machine * machine);

void
jive_graphcut_add_instruction(jive_graphcut * cut, jive_node * instruction, const jive_machine * machine);

#endif
