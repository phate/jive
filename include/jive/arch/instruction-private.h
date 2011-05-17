#ifndef JIVE_ARCH_INSTRUCTION_PRIVATE_H
#define JIVE_ARCH_INSTRUCTION_PRIVATE_H

#include <jive/arch/instruction.h>

/* inheritable instruction node member functions */

void
_jive_instruction_node_init_simple(
	jive_instruction_node * self,
	struct jive_region * region,
	const jive_instruction_class * icls,
	struct jive_output * const operands[],
	const long immediates[]);

void
_jive_instruction_node_fini(jive_node * self);

char *
_jive_instruction_node_get_label(const jive_node * self);

const jive_node_attrs *
_jive_instruction_node_get_attrs(const jive_node * self);

jive_node *
_jive_instruction_node_create(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

bool
_jive_instruction_node_match_attrs(const jive_node * self, const jive_node_attrs * attrs);

const struct jive_resource_class *
_jive_instruction_node_get_aux_rescls(const jive_node * self);

#endif
