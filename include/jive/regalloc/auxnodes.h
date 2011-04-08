#ifndef JIVE_REGALLOC_AUXNODES_H
#define JIVE_REGALLOC_AUXNODES_H

/* auxiliary nodes that are generated during register allocation
(all instances are replaced with machine-specific instructions
at the end of register allocation, so these nodes are never visible
outside the register allocator). */

#include <jive/vsdg/node.h>

struct jive_resource_class;
struct jive_shaped_graph;
struct jive_transfer_instructions_factory;

typedef struct jive_aux_split_node_attrs jive_aux_split_node_attrs;

struct jive_aux_split_node_attrs {
	const struct jive_resource_class * in_class;
	const struct jive_resource_class * out_class;
};

struct jive_aux_split_node {
	jive_node base;
	jive_aux_split_node_attrs attrs;
};

typedef struct jive_aux_split_node jive_aux_split_node;
extern const jive_node_class JIVE_AUX_SPLIT_NODE;

jive_node *
jive_aux_split_node_create(struct jive_region * region,
	const jive_type * in_type,
	struct jive_output * in_origin,
	const struct jive_resource_class * in_class,
	const jive_type * out_type,
	const struct jive_resource_class * out_class);

static inline jive_aux_split_node *
jive_aux_split_node_cast(jive_node * self)
{
	if (self->class_ == &JIVE_AUX_SPLIT_NODE) return (jive_aux_split_node *) self;
	else return 0;
}

void
jive_regalloc_auxnodes_replace(struct jive_shaped_graph * shaped_graph, const struct jive_transfer_instructions_factory * gen);

#if 0

typedef struct jive_aux_valuecopy_node jive_aux_valuecopy_node;
typedef struct jive_aux_spill_node jive_aux_spill_node;
typedef struct jive_aux_restore_node jive_aux_restore_node;
typedef struct jive_aux_node_attrs jive_aux_node_attrs;

struct jive_aux_node_attrs {
	jive_node_attrs base;
	const struct jive_regcls * regcls;
};

struct jive_aux_valuecopy_node {
	jive_node base;
	jive_aux_node_attrs attrs;
};

extern const jive_node_class JIVE_AUX_VALUECOPY_NODE;

struct jive_aux_spill_node {
	jive_node base;
	jive_aux_node_attrs attrs;
};

extern const jive_node_class JIVE_AUX_SPILL_NODE;

struct jive_aux_restore_node {
	jive_node base;
	jive_aux_node_attrs attrs;
};

extern const jive_node_class JIVE_AUX_RESTORE_NODE;

jive_node *
jive_aux_valuecopy_node_create(struct jive_region * region, const struct jive_regcls * regcls, struct jive_output * origin);

jive_node *
jive_aux_spill_node_create(struct jive_region * region, const struct jive_regcls * regcls, struct jive_output * origin);

jive_node *
jive_aux_restore_node_create(struct jive_region * region, const struct jive_regcls * regcls, struct jive_output * stackslot);

#endif

#endif
