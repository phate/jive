/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_INSTRUCTION_PRIVATE_H
#define JIVE_ARCH_INSTRUCTION_PRIVATE_H

#include <jive/arch/instruction.h>

/* inheritable instruction node member functions */

char *
jive_instruction_node_get_label_(const jive_node * self);

const jive_node_attrs *
jive_instruction_node_get_attrs_(const jive_node * self);

jive_node *
jive_instruction_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

bool
jive_instruction_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs);

const struct jive_resource_class *
jive_instruction_node_get_aux_rescls_(const jive_node * self);

#endif
