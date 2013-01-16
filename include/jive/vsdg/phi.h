/*
 * Copyright 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_PHI_H
#define JIVE_VSDG_PHI_H

#include <jive/vsdg/node.h>
#include <jive/vsdg/anchor.h>

/* phi node normal form */

typedef struct jive_anchor_node_normal_form jive_phi_node_normal_form;
typedef struct jive_phi_node_normal_form_class jive_phi_node_normal_form_class;

extern const jive_phi_node_normal_form_class JIVE_PHI_NODE_NORMAL_FORM_;
#define JIVE_PHI_NODE_NORMAL_FORM (JIVE_PHI_NODE_NORMAL_FORM_.base.base)

struct jive_phi_node_normal_form_class {
	jive_anchor_node_normal_form_class base;
	void (*normalized_create)(const jive_phi_node_normal_form * self,
		jive_region * phi_region, jive_output * results[]);
};

JIVE_EXPORTED_INLINE jive_phi_node_normal_form *
jive_phi_node_normal_form_cast(jive_node_normal_form * self)
{
	if (jive_node_normal_form_isinstance(self, &JIVE_PHI_NODE_NORMAL_FORM))
		return (jive_phi_node_normal_form *) self;
	else
		return NULL;
}

/* phi node */

extern const jive_node_class JIVE_PHI_NODE;
extern const jive_node_class JIVE_PHI_ENTER_NODE;
extern const jive_node_class JIVE_PHI_LEAVE_NODE;

typedef struct jive_phi_node jive_phi_node;
typedef struct jive_phi_node_attrs jive_phi_node_attrs;

struct jive_phi_node_attrs {
	jive_node_attrs base;
	jive_gate ** gates;
};

struct jive_phi_node {
	jive_node base;
	jive_phi_node_attrs attrs;
};

//struct jive_region *
//jive_phi_region_create(struct jive_region * region);

struct jive_region *
jive_phi_region_create(struct jive_region * parent,
	size_t narguments, const struct jive_type * argument_types[], struct jive_output * arguments[]);

struct jive_output *
jive_phi_region_finalize(struct jive_region * phi_region,
	size_t nreturns, struct jive_output * returns[]);

void
jive_phi_create(struct jive_region * phi_region, struct jive_output * results[]);

JIVE_EXPORTED_INLINE jive_phi_node *
jive_phi_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_PHI_NODE))
		return (jive_phi_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_node *
jive_phi_node_get_enter_node(const jive_phi_node * self)
{
	return self->base.inputs[0]->origin->node->region->top;
}

JIVE_EXPORTED_INLINE jive_node *
jive_phi_node_get_leave_node(const jive_phi_node * self)
{
	return self->base.inputs[0]->origin->node;
}

#endif
