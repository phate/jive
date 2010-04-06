#ifndef JIVE_MACHINE_H
#define JIVE_MACHINE_H

#include <jive/types.h>
#include <jive/buffer.h>

struct jive_graph;

typedef struct jive_cpureg jive_cpureg;
typedef struct jive_cpureg_class jive_cpureg_class;

#define MAX_REGISTER_CLASSES sizeof(jive_cpureg_classmask_t)*8

typedef short jive_regcls_count[MAX_REGISTER_CLASSES];

typedef enum {
	jive_instruction_flags_none = 0,
	/* instruction reuses first input register as output */
	jive_instruction_write_input = 1,
	/* first two input operands are commutative */
	jive_instruction_commutative = (1<<8)
} jive_instruction_flags;

typedef enum {
	jive_encode_out_of_memory = -1,
	jive_encode_ok = 0,
	jive_encode_next_pass = 1
} jive_encode_result;

struct jive_instruction_class {
	/** \brief Descriptive name of instruction, probably mnemonic name */
	const char *name;
	
	/**
		\brief Generate code
		\param target Target buffer to put
		\param instruction Instruction to encode
		\returns Encoding success
	*/
	jive_encode_result (*encode)(
		const jive_instruction_class * icls,
		jive_buffer * target,
		const jive_cpureg * inputs[],
		const jive_cpureg * outputs[],
		const long immediates[]);
	
	jive_encode_result (*mnemonic)(
		const jive_instruction_class * icls,
		jive_buffer * target,
		const jive_cpureg * inputs[],
		const jive_cpureg * outputs[],
		const long immediates[]);
	
	const jive_cpureg_class * const * inregs;
	const jive_cpureg_class * const * outregs;
	
	jive_instruction_flags flags;
	
	unsigned short ninputs;
	unsigned short noutputs;
	unsigned short nimmediates;
	
	/** \brief Internal number, used for code generation */
	int code;
};

struct jive_cpureg_class {
	/** \brief Descriptive name of register class */
	const char name[32];
	unsigned short nbits;
	
	const jive_cpureg * regs;
	unsigned short nregs;
	
	unsigned short index;
	const jive_cpureg_class * parent;
	jive_cpureg_classmask_t class_mask;
};

struct jive_cpureg {
	/** \brief Descriptive name of register, probably mnemonic name */
	const char name[32];
	const jive_cpureg_class * regcls;
	unsigned short code;
	unsigned short index;
	jive_cpureg_classmask_t class_mask;
};

struct jive_machine {
	const char name[32];
	const jive_cpureg * regs;
	const jive_cpureg_class * regcls;
	unsigned short nregs;
	unsigned short nregcls;
	
	const jive_regcls_count regcls_budget;
	
	jive_node * (*spill)(const jive_machine * machine, jive_value * value, jive_stackslot * where, jive_stackframe * frame);
	jive_value * (*restore)(const jive_machine * machine, struct jive_graph * graph, jive_stackslot * where, jive_stackframe * frame);
	struct jive_instruction * (*transfer)(const jive_machine * machine, struct jive_value * in, struct jive_value ** out);
};

/* intersection of register classes: returns the class representing the
intersection of registers in both classes (or NULL if the classes are
disjoint) */
const jive_cpureg_class *
jive_cpureg_class_intersect(const jive_cpureg_class * c1, const jive_cpureg_class * c2);

bool
jive_cpureg_class_contains(const jive_cpureg_class * superior, const jive_cpureg_class * inferior);

static inline void
jive_regcls_count_init(jive_regcls_count count)
{
	size_t n;
	for(n=0; n<MAX_REGISTER_CLASSES; n++) count[n] = 0;
}

static inline void
jive_regcls_count_copy(jive_regcls_count dst, const jive_regcls_count src)
{
	size_t n;
	for(n=0; n<MAX_REGISTER_CLASSES; n++) dst[n] = src[n];
}

static inline void
jive_regcls_count_add(jive_regcls_count count, jive_cpureg_class_t regcls)
{
	jive_cpureg_classmask_t mask = regcls->class_mask;
	size_t n = 0;
	while(mask) {
		if (mask & 1) count[n] ++;
		mask>>=1;
		n ++;
	}
}

static inline void
jive_regcls_count_sub(jive_regcls_count count, jive_cpureg_class_t regcls)
{
	jive_cpureg_classmask_t mask = regcls->class_mask;
	size_t n = 0;
	while(mask) {
		if (mask & 1) count[n] --;
		mask>>=1;
		n ++;
	}
}

static inline bool
jive_regcls_count_exceeds(const jive_regcls_count count, const jive_regcls_count budget)
{
	size_t n;
	for(n=0; n<MAX_REGISTER_CLASSES; n++)
		if (count[n] > budget[n]) return true;
	return false;
}

static inline bool
jive_regcls_count_plus_count_exceeds(const jive_regcls_count count1, const jive_regcls_count count2, const jive_regcls_count budget)
{
	size_t n;
	for(n=0; n<MAX_REGISTER_CLASSES; n++)
		if (count1[n] + count2[n] > budget[n]) return true;
	return false;
}

#endif
