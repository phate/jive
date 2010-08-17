#ifndef JIVE_REGALLOC_AUXNODES_H
#define JIVE_REGALLOC_AUXNODES_H

/* auxiliary nodes that are generated during register allocation
(all instances are replaced with machine-specific instructions
at the end of register allocation, so these nodes are never visible
outside the register allocator). */

#include <jive/vsdg/node.h>

struct jive_regcls;
struct jive_transfer_instructions_factory;

typedef struct jive_aux_valuecopy_node jive_aux_valuecopy_node;

struct jive_aux_valuecopy_node {
	jive_node base;
	const struct jive_regcls * regcls;
};

extern const jive_node_class JIVE_AUX_VALUECOPY_NODE;

jive_node *
jive_aux_valuecopy_node_create(struct jive_region * region, const struct jive_regcls * regcls, struct jive_output * origin);

void
jive_regalloc_auxnodes_replace(struct jive_graph * graph, const struct jive_transfer_instructions_factory * gen);

#endif
