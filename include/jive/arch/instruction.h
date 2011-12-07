#ifndef JIVE_ARCH_INSTRUCTION_H
#define JIVE_ARCH_INSTRUCTION_H

#include <string.h>

#include <jive/context.h>
#include <jive/arch/registers.h>
#include <jive/bitstring/type.h>
#include <jive/vsdg/label.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/sequence.h>

struct jive_buffer;
struct jive_label;

typedef struct jive_instruction_class jive_instruction_class;
typedef struct jive_instruction jive_instruction;
typedef struct jive_instruction_node jive_instruction_node;
typedef struct jive_instruction_node_attrs jive_instruction_node_attrs;
typedef struct jive_immediate jive_immediate;

typedef struct jive_seq_instruction jive_seq_instruction;

struct jive_immediate {
	jive_addr offset;
	const struct jive_label * add_label;
	const struct jive_label * sub_label;
	const void * modifier;
};

JIVE_EXPORTED_INLINE void
jive_immediate_init(jive_immediate * self, jive_addr offset, const struct jive_label * add_label, const struct jive_label * sub_label, const void * modifier)
{
	self->offset = offset;
	self->add_label = add_label;
	self->sub_label = sub_label;
	self->modifier = modifier;
}

JIVE_EXPORTED_INLINE jive_immediate
jive_immediate_add(const jive_immediate * a, const jive_immediate * b)
{
	jive_immediate tmp;
	const struct jive_label * add1 = a->add_label;
	const struct jive_label * add2 = b->add_label;
	const struct jive_label * sub1 = a->sub_label;
	const struct jive_label * sub2 = b->sub_label;
	
	if (add1 == sub2) {
		add1 = 0;
		sub2 = 0;
	}
	
	if (add2 && sub1) {
		add2 = 0;
		sub1 = 0;
	}
	
	jive_addr offset = a->offset + b->offset;
	
	if ((add1 && add2) || (sub1 && sub2) || (a->modifier || b->modifier)) {
		jive_immediate_init(&tmp, (jive_addr) -1,
			(const struct jive_label *) -1,
			(const struct jive_label *) -1,
			(const void *) -1);
	} else {
		jive_immediate_init(&tmp, offset, add1 ? add1 : add2, sub1 ? sub1 : sub2, NULL);
	}
	
	return tmp;
}

JIVE_EXPORTED_INLINE jive_immediate
jive_immediate_sub(const jive_immediate * a, const jive_immediate * b)
{
	jive_immediate tmp;
	const struct jive_label * add1 = a->add_label;
	const struct jive_label * add2 = b->sub_label;
	const struct jive_label * sub1 = a->sub_label;
	const struct jive_label * sub2 = b->add_label;
	
	if (add1 == sub2) {
		add1 = 0;
		sub2 = 0;
	}
	
	if (add2 && sub1) {
		add2 = 0;
		sub1 = 0;
	}
	
	jive_addr offset = a->offset - b->offset;
	
	if ((add1 && add2) || (sub1 && sub2) || (a->modifier || b->modifier)) {
		jive_immediate_init(&tmp, (jive_addr) -1,
			(const struct jive_label *) -1,
			(const struct jive_label *) -1,
			(const void *) -1);
	} else {
		jive_immediate_init(&tmp, offset, add1 ? add1 : add2, sub1 ? sub1 : sub2, NULL);
	}
	
	return tmp;
}

JIVE_EXPORTED_INLINE jive_immediate
jive_immediate_add_offset(jive_immediate * self, jive_addr offset)
{
	jive_immediate tmp = *self;
	tmp.offset += offset;
	return tmp;
}

JIVE_EXPORTED_INLINE bool
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

typedef enum {
	jive_instruction_encoding_flags_none = 0,
	/* if branch, primary target is not subsequent instruction */
	jive_instruction_encoding_flags_primary_branch_target = (1<<1),
	/* if branch, secondary target is not subsequent instruction */
	jive_instruction_encoding_flags_secondary_branch_target = (1<<2),
	
	/* the following flags may be updated by instruction encoding itself
	
	instructions may (depending on label distance etc.) have to choose
	between different displacement sizes, depending on labels; the idea is
	to start out conservative, but allow the instruction to "expand" its
	encoded size in case it does not fit (but never shrink) */
	
	jive_instruction_encoding_flags_option0 = (1<<16),
	jive_instruction_encoding_flags_option1 = (1<<17),
	jive_instruction_encoding_flags_option2 = (1<<18),
	jive_instruction_encoding_flags_option3 = (1<<19),
	jive_instruction_encoding_flags_option4 = (1<<20),
	jive_instruction_encoding_flags_option5 = (1<<21),
	jive_instruction_encoding_flags_option6 = (1<<22),
	jive_instruction_encoding_flags_option7 = (1<<23),
} jive_instruction_encoding_flags;

struct jive_instruction_class {
	/** \brief Descriptive name of instruction */
	const char * name;
	
	/** \brief Mnemonic name of instruction */
	const char * mnemonic;
	
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
		const jive_immediate immediates[],
		jive_instruction_encoding_flags * flags);
	
	/**
		\brief Generate mnemonic
		\param target Target buffer to put mnemonic instruction into
		\param instruction Instruction to encode
	*/
	void (*write_asm)(
		const jive_instruction_class * icls,
		struct jive_buffer * target,
		const jive_register_name * inputs[],
		const jive_register_name * outputs[],
		const jive_immediate immediates[],
		jive_instruction_encoding_flags * flags);
	
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

struct jive_instruction {
	const jive_instruction_class * icls;
	const jive_register_name ** inputs;
	const jive_register_name ** outputs;
	jive_immediate * immediates;
};

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

JIVE_EXPORTED_INLINE jive_node *
jive_instruction_node_create(
	struct jive_region * region,
	const jive_instruction_class * icls,
	struct jive_output * operands[const],
	const long immediates[const])
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

struct jive_seq_instruction {
	jive_seq_point base;
	jive_instruction instr;
	uint32_t flags;
};

extern const jive_seq_point_class JIVE_SEQ_INSTRUCTION;

jive_seq_instruction *
jive_seq_instruction_create(
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

/* FIXME: this is a placeholder function, will be replaced by a more
sophisticated interface later */
void
jive_graph_generate_code(struct jive_graph * graph, struct jive_buffer * buffer);

void
jive_graph_generate_assembler(struct jive_graph * graph, struct jive_buffer * buffer);

void
jive_seq_graph_generate_code(struct jive_seq_graph * seq_graph, struct jive_buffer * buffer);

void
jive_seq_graph_generate_assembler(struct jive_seq_graph * seq_graph, struct jive_buffer * buffer);

#endif
