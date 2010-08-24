#include <jive/backend/i386/registerset.h>
#include <jive/backend/i386/instructionset.h>

#include <jive/util/buffer.h>

#include <jive/debug-private.h>

#include <stdint.h>

static inline uint32_t
cpu_to_le32(uint32_t value)
{
	/* FIXME: endianness */
	return value;
}

static void
jive_i386_encode_simple(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_cpureg * inputs[],
	const jive_cpureg * outputs[],
	const long immediates[])
{
	jive_buffer_putbyte(target, icls->code);
}

static void
jive_i386_encode_int_load_imm(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_cpureg * inputs[],
	const jive_cpureg * outputs[],
	const long immediates[])
{
	int reg = outputs[0]->code;
	jive_buffer_putbyte(target, icls->code|reg);
	uint32_t immediate = cpu_to_le32(immediates[0]);
	jive_buffer_put(target, &immediate, 4);
}

static inline void
jive_i386_r2i(const jive_cpureg * r1, const jive_cpureg * r2, uint32_t displacement, jive_buffer * target)
{
	int regcode = (r1->code) | (r2->code<<3);
	/* special treatment for load/store through ebp: always encode displacement parameter */
	bool code_displacement = displacement || (r1->code==5);
	bool large_displacement = (displacement>127) || (displacement<-128);
	if (code_displacement) {
		if (large_displacement) regcode |= 0x80;
		else regcode |= 0x40;
	}
	
	jive_buffer_putbyte(target, regcode);
	if (r1->code == 4) {
		/* esp special treatment */
		jive_buffer_putbyte(target, 0x24);
	}
	
	if (code_displacement) {
		if (large_displacement) {
			displacement = cpu_to_le32(displacement);
			jive_buffer_put(target, &displacement, 4);
		} else jive_buffer_putbyte(target, displacement);
	}
}

static void
jive_i386_encode_loadstore32_disp(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_cpureg * inputs[],
	const jive_cpureg * outputs[],
	const long immediates[])
{
	uint32_t displacement = immediates[0];
	const jive_cpureg * r1 = inputs[0], * r2;
	if (icls->code == 0x89)
		r2 = inputs[1];
	else
		r2 = outputs[0];
	
	jive_buffer_putbyte(target, icls->code);
	
	jive_i386_r2i(r1, r2, displacement, target);
}

static void
jive_i386_encode_regreg(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_cpureg * inputs[],
	const jive_cpureg * outputs[],
	const long immediates[])
{
	int r1 = inputs[0]->code;
	int r2 = inputs[1]->code;
	
	DEBUG_ASSERT(r1 == outputs[0]->code);
	
	jive_buffer_putbyte(target, icls->code);
	jive_buffer_putbyte(target, 0xc0|r1|(r2<<3));
}

static void
jive_i386_encode_mul_regreg(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_cpureg * inputs[],
	const jive_cpureg * outputs[],
	const long immediates[])
{
	int r1 = inputs[0]->code;
	int r2 = inputs[1]->code;
	
	DEBUG_ASSERT(r1 == outputs[0]->code);
	
	jive_buffer_putbyte(target, 0x0f);
	jive_buffer_putbyte(target, 0xaf);
	jive_buffer_putbyte(target, 0xc0|r2|(r1<<3));
}

static void
jive_i386_encode_regimm(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_cpureg * inputs[],
	const jive_cpureg * outputs[],
	const long immediates[])
{
	int r1 = inputs[0]->code;
	DEBUG_ASSERT(r1 == outputs[0]->code);
	int32_t immediate = immediates[0];
	
	bool long_form = (immediate>127) || (immediate<-128);
	
	char prefix = long_form ? 0x81 : 0x83;
	
	if (r1 == 0) {
		char opcode = icls->code >> 8;
		jive_buffer_putbyte(target, opcode);
	} else {
		char opcode = icls->code & 255;
		jive_buffer_putbyte(target, prefix);
		jive_buffer_putbyte(target, opcode | r1);
	}
	
	if (long_form) {
		immediate = cpu_to_le32(immediate);
		jive_buffer_put(target, &immediate, 4);
	} else jive_buffer_putbyte(target, immediate);
}

static void
jive_i386_encode_unaryreg(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_cpureg * inputs[],
	const jive_cpureg * outputs[],
	const long immediates[])
{
	int r1 = inputs[0]->code;
	
	DEBUG_ASSERT(r1 == outputs[0]->code);
	
	jive_buffer_putbyte(target, 0xf7);
	jive_buffer_putbyte(target, icls->code|r1);
}

static void
jive_i386_encode_regmove(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_cpureg * inputs[],
	const jive_cpureg * outputs[],
	const long immediates[])
{
	int r1 = outputs[0]->code;
	int r2 = inputs[0]->code;
	
	jive_buffer_putbyte(target, icls->code);
	jive_buffer_putbyte(target, 0xc0|r1|(r2<<3));
}

static const jive_regcls * const intreg_param[] = {
	&jive_i386_regcls[jive_i386_gpr],
	&jive_i386_regcls[jive_i386_gpr],
	&jive_i386_regcls[jive_i386_gpr]
};

static const jive_regcls * const intflags_param[] = {
	&jive_i386_regcls[jive_i386_gpr],
	&jive_i386_regcls[jive_i386_flags]
};

const jive_instruction_class jive_i386_instructions[] = {
	[jive_i386_ret] = {
		.name = "ret",
		.encode = jive_i386_encode_simple,
		.mnemonic = 0,
		.inregs = 0, .outregs = 0, .flags = jive_instruction_flags_none, .ninputs = 0, .noutputs = 0, .nimmediates = 0,
		.code = 0xc3
	},
	
	[jive_i386_int_load_imm] = {
		.name = "int_load_imm",
		.encode = jive_i386_encode_int_load_imm,
		.mnemonic = 0,
		.inregs = 0, .outregs = intreg_param, .flags = jive_instruction_flags_none,
		.ninputs = 0, .noutputs = 1, .nimmediates = 1,
		.code = 0xb8
	},
	
	[jive_i386_int_load32_disp] = {
		.name = "int_load32_disp",
		.encode = jive_i386_encode_loadstore32_disp,
		.mnemonic = 0,
		.inregs = intreg_param, .outregs = intreg_param, .flags = jive_instruction_flags_none,
		.ninputs = 1, .noutputs = 1, .nimmediates = 1,
		.code = 0x8b
	},
	[jive_i386_int_store32_disp] = {
		.name = "int_store32_disp",
		.encode = jive_i386_encode_loadstore32_disp,
		.mnemonic = 0,
		.inregs = intreg_param, .outregs = 0, .flags = jive_instruction_flags_none,
		.ninputs = 2, .noutputs = 0, .nimmediates = 1,
		.code = 0x89
	},
	[jive_i386_int_add] = {
		.name = "int_add",
		.encode = jive_i386_encode_regreg,
		.mnemonic = 0,
		.inregs = intreg_param, .outregs = intflags_param, .flags = jive_instruction_write_input | jive_instruction_commutative,
		.ninputs = 2, .noutputs = 2, .nimmediates = 0,
		.code = 0x01
	},
	[jive_i386_int_sub] = {
		.name = "int_sub",
		.encode = jive_i386_encode_regreg,
		.mnemonic = 0,
		.inregs = intreg_param, .outregs = intflags_param, .flags = jive_instruction_write_input,
		.ninputs = 2, .noutputs = 2, .nimmediates = 0,
		.code = 0x29
	},
	[jive_i386_int_and] = {
		.name = "int_and",
		.encode = jive_i386_encode_regreg,
		.mnemonic = 0,
		.inregs = intreg_param, .outregs = intflags_param, .flags = jive_instruction_write_input | jive_instruction_commutative,
		.ninputs = 2, .noutputs = 2, .nimmediates = 0,
		.code = 0x21
	},
	[jive_i386_int_or] = {
		.name = "int_or",
		.encode = jive_i386_encode_regreg,
		.mnemonic = 0,
		.inregs = intreg_param, .outregs = intflags_param, .flags = jive_instruction_write_input | jive_instruction_commutative,
		.ninputs = 2, .noutputs = 2, .nimmediates = 0,
		.code = 0x09
	},
	[jive_i386_int_xor] = {
		.name = "int_xor",
		.encode = jive_i386_encode_regreg,
		.mnemonic = 0,
		.inregs = intreg_param, .outregs = intflags_param, .flags = jive_instruction_write_input | jive_instruction_commutative,
		.ninputs = 2, .noutputs = 2, .nimmediates = 0,
		.code = 0x31
	},
	[jive_i386_int_mul] = {
		.name = "int_mul",
		.encode = jive_i386_encode_mul_regreg,
		.mnemonic = 0,
		.inregs = intreg_param, .outregs = intflags_param, .flags = jive_instruction_write_input | jive_instruction_commutative,
		.ninputs = 2, .noutputs = 2, .nimmediates = 0,
		.code = 0xc0af0f
	},
	
	/* for the immediate instructions, code consists of normal_code | (eax_code << 8),
	see instruction coding function for explanation */
	[jive_i386_int_add_immediate] = {
		.name = "int_add_immediate",
		.encode = jive_i386_encode_regimm,
		.mnemonic = 0,
		.inregs = intreg_param, .outregs = intflags_param, .flags = jive_instruction_write_input,
		.ninputs = 1, .noutputs = 2, .nimmediates = 1,
		.code = 0xc0 | (0x05 << 8)
	},
	[jive_i386_int_sub_immediate] = {
		.name = "int_sub_immediate",
		.encode = jive_i386_encode_regimm,
		.mnemonic = 0,
		.inregs = intreg_param, .outregs = intflags_param, .flags = jive_instruction_write_input,
		.ninputs = 1, .noutputs = 2, .nimmediates = 1,
		.code = 0xe8 | (0x2d << 8)
	},
	[jive_i386_int_and_immediate] = {
		.name = "int_and_immediate",
		.encode = jive_i386_encode_regimm,
		.mnemonic = 0,
		.inregs = intreg_param, .outregs = intflags_param, .flags = jive_instruction_write_input,
		.ninputs = 1, .noutputs = 2, .nimmediates = 1,
		.code = 0xe0 | (0x25 << 8)
	},
	[jive_i386_int_or_immediate] = {
		.name = "int_or_immediate",
		.encode = jive_i386_encode_regimm,
		.mnemonic = 0,
		.inregs = intreg_param, .outregs = intflags_param, .flags = jive_instruction_write_input,
		.ninputs = 1, .noutputs = 2, .nimmediates = 1,
		.code = 0xc8 | (0x0d << 8)
	},
	[jive_i386_int_xor_immediate] = {
		.name = "int_xor_immediate",
		.encode = jive_i386_encode_regimm,
		.mnemonic = 0,
		.inregs = intreg_param, .outregs = intflags_param, .flags = jive_instruction_write_input,
		.ninputs = 1, .noutputs = 2, .nimmediates = 1,
		.code = 0xc0 | (0x3d << 8)
	},
	[jive_i386_int_neg] = {
		.name = "int_neg",
		.encode = jive_i386_encode_unaryreg,
		.mnemonic = 0,
		.inregs = intreg_param, .outregs = intflags_param, .flags = jive_instruction_write_input,
		.ninputs = 1, .noutputs = 2, .nimmediates = 0,
		.code = 0xd8
	},
	[jive_i386_int_not] = {
		.name = "int_not",
		.encode = jive_i386_encode_unaryreg,
		.mnemonic = 0,
		.inregs = intreg_param, .outregs = intflags_param, .flags = jive_instruction_write_input,
		.ninputs = 1, .noutputs = 2, .nimmediates = 0,
		.code = 0xd0
	},
	[jive_i386_int_transfer] = {
		.name = "int_transfer",
		.encode = jive_i386_encode_regmove,
		.mnemonic = 0,
		.inregs = intreg_param, .outregs = intreg_param,
		.ninputs = 1, .noutputs = 1, .nimmediates = 0,
		.code = 0x89
	},
};
