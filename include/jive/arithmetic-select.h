#ifndef JIVE_ARITHMETIC_SELECT_H
#define JIVE_ARITHMETIC_SELECT_H

struct jive_graph;
struct jive_machine;
struct jive_node;

/* for each abstract arithmetic operation in the graph,
choose a suitable register class where this operation
can be performed in */
void
jive_arithmetic_select(struct jive_graph * graph,
	const struct jive_machine * machine);

struct jive_node *
jive_arithmetic_transform_single(struct jive_node * _node, unsigned int width);

#endif
