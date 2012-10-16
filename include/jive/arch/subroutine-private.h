/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_SUBROUTINE_PRIVATE_H
#define JIVE_ARCH_SUBROUTINE_PRIVATE_H

#include <jive/arch/subroutine.h>

jive_node *
jive_subroutine_enter_node_create(struct jive_region * region);

jive_node *
jive_subroutine_leave_node_create(struct jive_region * region, struct jive_output * control_transfer);

jive_node *
jive_subroutine_node_create(struct jive_region * subroutine_region, jive_subroutine * subroutine);

void
jive_subroutine_create_region_and_nodes(jive_subroutine * subroutine, struct jive_region * parent_region);

jive_subroutine_passthrough
jive_subroutine_create_passthrough(jive_subroutine * subroutine, const struct jive_resource_class * cls, const char * name);

jive_gate *
jive_subroutine_match_gate(jive_gate * gate, jive_node * old_node, jive_node * new_node);

static inline void
jive_subroutine_match_passthrough(const jive_subroutine * old_subroutine, const jive_subroutine_passthrough * old_pt,
	jive_subroutine * new_subroutine, jive_subroutine_passthrough * new_pt)
{
	new_pt->output = new_subroutine->enter->base.outputs[old_pt->output->index];
	new_pt->input = new_subroutine->leave->base.inputs[old_pt->input->index];
	new_pt->gate = new_pt->output->gate;
}

/* base constructor */
void
jive_subroutine_init_(jive_subroutine * self, const jive_subroutine_class * cls, jive_context * context,
	const struct jive_instructionset * instructionset,
	size_t nparameters,
	size_t nreturns,
	size_t npassthroughs);

/* base destructor */
void
jive_subroutine_fini_(jive_subroutine * self);

#endif
