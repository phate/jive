#ifndef JIVE_ARCH_INSTRUCTION_H
#define JIVE_ARCH_INSTRUCTION_H

#include <jive/arch/registers.h>
#include <jive/vsdg/node.h>
#include <jive/bitstring/type.h>

struct jive_buffer;

typedef struct jive_instruction_class jive_instruction_class;
typedef struct jive_instruction_node jive_instruction_node;
typedef struct jive_instruction_node_attrs jive_instruction_node_attrs;

typedef enum {
	jive_instruction_flags_none = 0,
	/* instruction reuses first input register as output */
	jive_instruction_write_input = 1,
	/* first two input operands are commutative */
	jive_instruction_commutative = (1<<8)
} jive_instruction_flags;

struct jive_instruction_class {
	/** \brief Descriptive name of instruction, probably mnemonic name */
	const char * name;
	
	/**
		\brief Generate code
		\param target Target buffer to put encoded instructions into
		\param instruction Instruction to encode
	*/
	void (*encode)(
		const jive_instruction_class * icls,
		struct jive_buffer * target,
		const jive_cpureg * inputs[],
		const jive_cpureg * outputs[],
		const long immediates[]);
	
	/**
		\brief Generate mnemonic
		\param target Target buffer to put mnemonic instruction into
		\param instruction Instruction to encode
	*/
	void (*mnemonic)(
		const jive_instruction_class * icls,
		struct jive_buffer * target,
		const jive_cpureg * inputs[],
		const jive_cpureg * outputs[],
		const long immediates[]);
	
	const jive_regcls * const * inregs;
	const jive_regcls * const * outregs;
	
	jive_instruction_flags flags;
	
	unsigned short ninputs;
	unsigned short noutputs;
	unsigned short nimmediates;
	
	/** \brief Internal number, used for code generation */
	int code;
};

extern const jive_instruction_class JIVE_PSEUDO_NOP;

extern const jive_node_class JIVE_INSTRUCTION_NODE;

struct jive_instruction_node_attrs {
	jive_node_attrs base;
	const jive_instruction_class * icls;
	long * immediates;
};

struct jive_instruction_node {
	jive_node base;
	jive_instruction_node_attrs attrs;
};

jive_instruction_node *
jive_instruction_node_create(
	struct jive_region * region,
	const jive_instruction_class * icls,
	struct jive_output * operands[const],
	const long immediates[const]);

/* FIXME: this is a placeholder function, will be replaced by a more
sophisticated interface later */
void
jive_graph_generate_code(struct jive_graph * graph, struct jive_buffer * buffer);

#endif
