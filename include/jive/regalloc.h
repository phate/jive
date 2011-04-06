#ifndef JIVE_REGALLOC_H
#define JIVE_REGALLOC_H

struct jive_graph;
struct jive_shaped_graph;
struct jive_transfer_instructions_factory;

struct jive_shaped_graph *
jive_regalloc(struct jive_graph * graph, const struct jive_transfer_instructions_factory * xfer);

#endif
