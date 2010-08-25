#ifndef JIVE_ARCH_INSTRUCTION_PRIVATE_H
#define JIVE_ARCH_INSTRUCTION_PRIVATE_H

#include <jive/arch/instruction.h>

/* inheritable instruction node member functions */

void
_jive_instruction_node_init(
	jive_instruction_node * self,
	struct jive_region * region,
	const jive_instruction_class * icls,
	struct jive_output * operands[const],
	const long immediates[const]);

void
_jive_instruction_node_fini(jive_node * self);

char *
_jive_instruction_node_get_label(const jive_node * self);

const jive_node_attrs *
_jive_instruction_node_get_attrs(const jive_node * self);

jive_node *
_jive_instruction_node_create(const jive_node_attrs * attrs, struct jive_region * region,
	size_t noperands, struct jive_output * operands[]);

bool
_jive_instruction_node_equiv(const jive_node_attrs * first, const jive_node_attrs * second);

const struct jive_regcls *
_jive_instruction_node_get_aux_regcls(const jive_node * self);

#endif
