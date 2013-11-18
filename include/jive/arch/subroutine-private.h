/*
 * Copyright 2010 2011 2012 2013 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_SUBROUTINE_PRIVATE_H
#define JIVE_ARCH_SUBROUTINE_PRIVATE_H

#include <jive/arch/subroutine.h>

jive_node *
jive_subroutine_enter_node_create(struct jive_region * region);

jive_node *
jive_subroutine_leave_node_create(
	struct jive_region * region,
	struct jive_output * control_transfer);

jive_node *
jive_subroutine_node_create(
	struct jive_region * subroutine_region,
	jive_subroutine * subroutine);

void
jive_subroutine_create_region_and_nodes(
	jive_subroutine * subroutine,
	struct jive_region * parent_region);

jive_subroutine_passthrough
jive_subroutine_create_passthrough(
	jive_subroutine * subroutine,
	const struct jive_resource_class * cls,
	const char * name);

jive_gate *
jive_subroutine_match_gate(
	jive_gate * gate,
	jive_node * old_node,
	jive_node * new_node);

jive_subroutine *
jive_subroutine_copy(
	const jive_subroutine * self,
	jive_node * new_enter_node, jive_node * new_leave_node);

jive_subroutine *
jive_subroutine_create_takeover(
	jive_context * context, const jive_subroutine_class * class_,
	size_t nparameters, jive_gate * const parameters[],
	size_t nreturns, jive_gate * const returns[],
	size_t npassthroughs, const jive_subroutine_passthrough passthroughs[]);

void
jive_subroutine_match_passthrough(
	const jive_subroutine * old_subroutine,
	const jive_subroutine_passthrough * old_pt,
	jive_subroutine * new_subroutine,
	jive_subroutine_passthrough * new_pt);

/* base constructor */
void
jive_subroutine_init_(jive_subroutine * self, const jive_subroutine_class * cls,
	jive_context * context,
	size_t nparameters, const jive_argument_type parameter_types[],
	size_t nreturns, const jive_argument_type return_types[],
	size_t npassthroughs);

/* base destructor */
void
jive_subroutine_fini_(jive_subroutine * self);

#endif
