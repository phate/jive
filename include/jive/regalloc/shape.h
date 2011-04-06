#ifndef JIVE_REGALLOC_SHAPE_H
#define JIVE_REGALLOC_SHAPE_H

struct jive_graph;
struct jive_shaped_graph;

struct jive_shaped_graph *
jive_regalloc_shape(struct jive_graph * graph);

#endif
