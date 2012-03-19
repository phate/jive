#ifndef JIVE_REGALLOC_H
#define JIVE_REGALLOC_H

struct jive_graph;
struct jive_shaped_graph;

struct jive_shaped_graph *
jive_regalloc(struct jive_graph * graph);

#endif
