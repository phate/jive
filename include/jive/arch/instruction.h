/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_INSTRUCTION_H
#define JIVE_ARCH_INSTRUCTION_H

#include <string.h>

#include <jive/context.h>
#include <jive/arch/immediate-node.h>
#include <jive/arch/instruction-class.h>
#include <jive/arch/linker-symbol.h>
#include <jive/arch/registers.h>
#include <jive/arch/sequence.h>
#include <jive/types/bitstring/type.h>
#include <jive/vsdg/label.h>
#include <jive/vsdg/node.h>

typedef struct jive_instruction jive_instruction;
typedef struct jive_instruction_node jive_instruction_node;
typedef struct jive_instruction_node_attrs jive_instruction_node_attrs;
typedef struct jive_seq_instruction jive_seq_instruction;


extern const jive_node_class JIVE_INSTRUCTION_NODE;

struct jive_instruction_node_attrs {
	jive_node_attrs base;
	const jive_instruction_class * icls;
};

struct jive_instruction_node {
	jive_node base;
	jive_instruction_node_attrs attrs;
};

jive_node *
jive_instruction_node_create_simple(
	struct jive_region * region,
	const jive_instruction_class * icls,
	struct jive_output * operands[const],
	const int64_t immediates[const]);

jive_node *
jive_instruction_node_create_extended(
	struct jive_region * region,
	const jive_instruction_class * icls,
	jive_output * operands[const],
	const jive_immediate immediates[]);

JIVE_EXPORTED_INLINE jive_node *
jive_instruction_node_create(
	struct jive_region * region,
	const jive_instruction_class * icls,
	struct jive_output * operands[const],
	const int64_t immediates[const])
{
	return jive_instruction_node_create_simple(region, icls, operands, immediates);
}

JIVE_EXPORTED_INLINE jive_instruction_node *
jive_instruction_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_INSTRUCTION_NODE)
		return (jive_instruction_node *) node;
	else
		return 0;
}

struct jive_instruction {
	const jive_instruction_class * icls;
	const jive_register_name ** inputs;
	const jive_register_name ** outputs;
	jive_immediate * immediates;
};

struct jive_seq_instruction {
	jive_seq_point base;
	jive_instruction instr;
	uint32_t flags;
};

extern const jive_seq_point_class JIVE_SEQ_INSTRUCTION;

jive_seq_instruction *
jive_seq_instruction_create_before(
	jive_seq_point * before,
	const jive_instruction_class * icls,
	const jive_register_name * const * inputs,
	const jive_register_name * const * outputs,
	const jive_immediate immediates[]);

jive_seq_instruction *
jive_seq_instruction_create_after(
	jive_seq_point * before,
	const jive_instruction_class * icls,
	const jive_register_name * const * inputs,
	const jive_register_name * const * outputs,
	const jive_immediate immediates[]);

JIVE_EXPORTED_INLINE jive_seq_instruction *
jive_seq_instruction_cast(jive_seq_point * self)
{
	if (self->class_ == &JIVE_SEQ_INSTRUCTION)
		return (jive_seq_instruction *) self;
	else
		return 0;
}

#endif
