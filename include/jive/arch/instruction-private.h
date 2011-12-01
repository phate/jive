#ifndef JIVE_ARCH_INSTRUCTION_PRIVATE_H
#define JIVE_ARCH_INSTRUCTION_PRIVATE_H

#include <jive/arch/instruction.h>

/* inheritable instruction node member functions */

void
jive_instruction_node_init_simple_(
	jive_instruction_node * self,
	struct jive_region * region,
	const jive_instruction_class * icls,
	struct jive_output * const operands[],
	const int64_t immediates[]);

void
jive_instruction_node_fini_(jive_node * self);

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
