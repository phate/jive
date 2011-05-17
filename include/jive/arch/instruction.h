#ifndef JIVE_ARCH_INSTRUCTION_H
#define JIVE_ARCH_INSTRUCTION_H

#include <string.h>

#include <jive/context.h>
#include <jive/arch/registers.h>
#include <jive/vsdg/node.h>
#include <jive/bitstring/type.h>

struct jive_buffer;
struct jive_asm_label;

typedef struct jive_instruction_class jive_instruction_class;
typedef struct jive_instruction_node jive_instruction_node;
typedef struct jive_instruction_node_attrs jive_instruction_node_attrs;
typedef struct jive_immediate jive_immediate;

struct jive_immediate {
	unsigned long long offset;
	const struct jive_asm_label * add_label;
	const struct jive_asm_label * sub_label;
	const void * modifier;
};

static inline void
jive_immediate_init(jive_immediate * self, unsigned long long offset, const struct jive_asm_label * add_label, const struct jive_asm_label * sub_label, const void * modifier)
{
	self->offset = offset;
	self->add_label = add_label;
	self->sub_label = sub_label;
	self->modifier = modifier;
}

static inline jive_immediate
jive_immediate_add(const jive_immediate * a, const jive_immediate * b)
{
	jive_immediate tmp;
	const struct jive_asm_label * add1 = a->add_label;
	const struct jive_asm_label * add2 = b->add_label;
	const struct jive_asm_label * sub1 = a->sub_label;
	const struct jive_asm_label * sub2 = b->sub_label;
	
	if (add1 == sub2) {
		add1 = 0;
		sub2 = 0;
	}
	
	if (add2 && sub1) {
		add2 = 0;
		sub1 = 0;
	}
	
	unsigned long long offset = a->offset + b->offset;
	
	if ((add1 && add2) || (sub1 && sub2) || (a->modifier || b->modifier)) {
		jive_immediate_init(&tmp, (unsigned long long) -1,
			(const struct jive_asm_label *) -1,
			(const struct jive_asm_label *) -1,
			(const void *) -1);
	} else {
		jive_immediate_init(&tmp, offset, add1 ? add1 : add2, sub1 ? sub1 : sub2, NULL);
	}
	
	return tmp;
}

static inline jive_immediate
jive_immediate_sub(const jive_immediate * a, const jive_immediate * b)
{
	jive_immediate tmp;
	const struct jive_asm_label * add1 = a->add_label;
	const struct jive_asm_label * add2 = b->sub_label;
	const struct jive_asm_label * sub1 = a->sub_label;
	const struct jive_asm_label * sub2 = b->add_label;
	
	if (add1 == sub2) {
		add1 = 0;
		sub2 = 0;
	}
	
	if (add2 && sub1) {
		add2 = 0;
		sub1 = 0;
	}
	
	unsigned long long offset = a->offset - b->offset;
	
	if ((add1 && add2) || (sub1 && sub2) || (a->modifier || b->modifier)) {
		jive_immediate_init(&tmp, (unsigned long long) -1,
			(const struct jive_asm_label *) -1,
			(const struct jive_asm_label *) -1,
			(const void *) -1);
	} else {
		jive_immediate_init(&tmp, offset, add1 ? add1 : add2, sub1 ? sub1 : sub2, NULL);
	}
	
	return tmp;
}

static inline jive_immediate
jive_immediate_add_offset(jive_immediate * self, unsigned long long offset)
{
	jive_immediate tmp = *self;
	tmp.offset += offset;
	return tmp;
}

static inline bool
jive_immediate_equals(const jive_immediate * self, const jive_immediate * other)
{
	return 
		(self->offset == other->offset) &&
		(self->add_label == other->add_label) &&
		(self->sub_label == other->sub_label) &&
		(self->modifier == other->modifier);
}

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
		const jive_register_name * inputs[],
		const jive_register_name * outputs[],
		const jive_immediate immediates[]);
	
	/**
		\brief Generate mnemonic
		\param target Target buffer to put mnemonic instruction into
		\param instruction Instruction to encode
	*/
	void (*mnemonic)(
		const jive_instruction_class * icls,
		struct jive_buffer * target,
		const jive_register_name * inputs[],
		const jive_register_name * outputs[],
		const jive_immediate immediates[]);
	
	const jive_register_class * const * inregs;
	const jive_register_class * const * outregs;
	
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
	jive_immediate * immediates;
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
	const long immediates[const]);

jive_node *
jive_instruction_node_create_extended(
	struct jive_region * region,
	const jive_instruction_class * icls,
	jive_output * operands[const],
	const jive_immediate immediates[]);

static inline jive_node *
jive_instruction_node_create(
	struct jive_region * region,
	const jive_instruction_class * icls,
	struct jive_output * operands[const],
	const long immediates[const])
{
	return jive_instruction_node_create_simple(region, icls, operands, immediates);
}

/* FIXME: this is a placeholder function, will be replaced by a more
sophisticated interface later */
void
jive_graph_generate_code(struct jive_graph * graph, struct jive_buffer * buffer);

#endif
