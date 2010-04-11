#include <jive/i386/machine.h>

#include "encoding.h"

static jive_encode_result
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
	
	if (!jive_buffer_putbyte(target, icls->code))
		return jive_encode_out_of_memory;
	
	return jive_i386_r2i(r1, r2, displacement, target);
}

static const jive_cpureg_class * const intreg_param[] = {
	&jive_i386_regcls[jive_i386_gpr],
	&jive_i386_regcls[jive_i386_gpr],
	&jive_i386_regcls[jive_i386_gpr]
};

static const jive_cpureg_class * const intflags_param[] = {
	&jive_i386_regcls[jive_i386_gpr],
	&jive_i386_regcls[jive_i386_flags]
};

const jive_instruction_class jive_i386_instructions[] = {
	[jive_i386_ret] = {
		.name = "ret",
		.encode = &jive_i386_encode_simple,
		.mnemonic = 0,
		.inregs = 0, .outregs = 0, .flags = jive_instruction_flags_none, .ninputs = 0, .noutputs = 0, .nimmediates = 0,
		.code = 0xc3
	},
	
	[jive_i386_int_load_imm] = {
		.name = "int_load_imm",
		.encode = &jive_i386_encode_int_load_imm,
		.mnemonic = 0,
		.inregs = 0, .outregs = intreg_param, .flags = jive_instruction_flags_none,
		.ninputs = 0, .noutputs = 1, .nimmediates = 1,
		.code = 0xb8
	},
	
	[jive_i386_int_load32_disp] = {
		.name = "int_load32_disp",
		.encode = &jive_i386_encode_loadstore32_disp,
		.mnemonic = 0,
		.inregs = intreg_param, .outregs = intreg_param, .flags = jive_instruction_flags_none,
		.ninputs = 1, .noutputs = 1, .nimmediates = 1,
		.code = 0x8b
	},
	[jive_i386_int_store32_disp] = {
		.name = "int_store32_disp",
		.encode = &jive_i386_encode_loadstore32_disp,
		.mnemonic = 0,
		.inregs = intreg_param, .outregs = 0, .flags = jive_instruction_flags_none,
		.ninputs = 2, .noutputs = 0, .nimmediates = 1,
		.code = 0x89
	},
	
	[jive_i386_int_add] = {
		.name = "int_add",
		.encode = &jive_i386_encode_regreg,
		.mnemonic = 0,
		.inregs = intreg_param, .outregs = intflags_param, .flags = jive_instruction_write_input | jive_instruction_commutative,
		.ninputs = 2, .noutputs = 2, .nimmediates = 0,
		.code = 0x01
	},
	[jive_i386_int_sub] = {
		.name = "int_sub",
		.encode = &jive_i386_encode_regreg,
		.mnemonic = 0,
		.inregs = intreg_param, .outregs = intflags_param, .flags = jive_instruction_write_input,
		.ninputs = 2, .noutputs = 2, .nimmediates = 0,
		.code = 0x29
	},
	[jive_i386_int_and] = {
		.name = "int_and",
		.encode = &jive_i386_encode_regreg,
		.mnemonic = 0,
		.inregs = intreg_param, .outregs = intflags_param, .flags = jive_instruction_write_input | jive_instruction_commutative,
		.ninputs = 2, .noutputs = 2, .nimmediates = 0,
		.code = 0x21
	},
	[jive_i386_int_or] = {
		.name = "int_or",
		.encode = &jive_i386_encode_regreg,
		.mnemonic = 0,
		.inregs = intreg_param, .outregs = intflags_param, .flags = jive_instruction_write_input | jive_instruction_commutative,
		.ninputs = 2, .noutputs = 2, .nimmediates = 0,
		.code = 0x09
	},
	[jive_i386_int_xor] = {
		.name = "int_xor",
		.encode = &jive_i386_encode_regreg,
		.mnemonic = 0,
		.inregs = intreg_param, .outregs = intflags_param, .flags = jive_instruction_write_input | jive_instruction_commutative,
		.ninputs = 2, .noutputs = 2, .nimmediates = 0,
		.code = 0x31
	},
	[jive_i386_int_mul] = {
		.name = "int_mul",
		.encode = &jive_i386_encode_mul_regreg,
		.mnemonic = 0,
		.inregs = intreg_param, .outregs = intflags_param, .flags = jive_instruction_write_input | jive_instruction_commutative,
		.ninputs = 2, .noutputs = 2, .nimmediates = 0,
		.code = 0xc0af0f
	},
	[jive_i386_int_transfer] = {
		.name = "int_transfer",
		.encode = &jive_i386_encode_regmove,
		.mnemonic = 0,
		.inregs = intreg_param, .outregs = intreg_param,
		.ninputs = 1, .noutputs = 1, .nimmediates = 0,
		.code = 0x89
	},
};
