#include <jive/arch/instructionset.h>
#include <jive/backend/i386/classifier.h>
#include <jive/backend/i386/registerset.h>
#include <jive/backend/i386/instructionset.h>
#include <jive/backend/i386/machine.h>

#include <jive/util/buffer.h>

#include <stdio.h>
#include <stdint.h>

static inline uint32_t
cpu_to_le32(uint32_t value)
{
	/* FIXME: endianness */
	return value;
}

static void
jive_buffer_putimm(jive_buffer * target, const jive_immediate * imm)
{
	bool empty = true;
	if (imm->offset) {
		char tmp[80];
		snprintf(tmp, sizeof(tmp), "%lld", imm->offset);
		jive_buffer_putstr(target, tmp);
		empty = false;
	}
	if (imm->add_label) {
		if (!empty)
			jive_buffer_putstr(target, "+");
		jive_buffer_putstr(target, jive_label_get_asmname(imm->add_label));
		empty = false;
	}
	if (imm->sub_label) {
		jive_buffer_putstr(target, "-");
		jive_buffer_putstr(target, jive_label_get_asmname(imm->sub_label));
		empty = false;
	}
	if (empty) {
		jive_buffer_putstr(target, "0");
	}
}

static void
jive_buffer_putdisp(jive_buffer * target, const jive_immediate * disp, const jive_register_name * reg)
{
	jive_buffer_putimm(target, disp);
	jive_buffer_putstr(target, "(%");
	jive_buffer_putstr(target, reg->base.name);
	jive_buffer_putstr(target, ")");
}

static void
jive_i386_encode_simple(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_immediate immediates[],
	jive_instruction_encoding_flags * flags)
{
	jive_buffer_putbyte(target, icls->code);
}

static void
jive_i386_asm_simple(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_immediate immediates[],
	jive_instruction_encoding_flags * flags)
{
	jive_buffer_putstr(target, icls->mnemonic);
}

static void
jive_i386_encode_int_load_imm(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_immediate immediates[],
	jive_instruction_encoding_flags * flags)
{
	int reg = outputs[0]->code;
	jive_buffer_putbyte(target, icls->code|reg);
	uint32_t immediate = cpu_to_le32(immediates[0].offset);
	jive_buffer_put(target, &immediate, 4);
}

static void
jive_i386_asm_int_load_imm(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_immediate immediates[],
	jive_instruction_encoding_flags * flags)
{
	jive_buffer_putstr(target, icls->mnemonic);
	jive_buffer_putstr(target, "\t$");
	jive_buffer_putimm(target, &immediates[0]);
	jive_buffer_putstr(target, ", %");
	jive_buffer_putstr(target, outputs[0]->base.name);
}

static inline void
jive_i386_r2i(const jive_register_name * r1, const jive_register_name * r2, uint32_t displacement, jive_buffer * target)
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
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_immediate immediates[],
	jive_instruction_encoding_flags * flags)
{
	uint32_t displacement = immediates[0].offset;
	const jive_register_name * r1 = inputs[0], * r2;
	if (icls->code == 0x89)
		r2 = inputs[1];
	else
		r2 = outputs[0];
	
	jive_buffer_putbyte(target, icls->code);
	
	jive_i386_r2i(r1, r2, displacement, target);
}

static void
jive_i386_asm_load(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_immediate immediates[],
	jive_instruction_encoding_flags * flags)
{
	jive_buffer_putstr(target, icls->mnemonic);
	jive_buffer_putstr(target, "\t");
	jive_buffer_putdisp(target, &immediates[0], inputs[0]);
	jive_buffer_putstr(target, ", %");
	jive_buffer_putstr(target, outputs[0]->base.name);
}

static void
jive_i386_asm_store(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_immediate immediates[],
	jive_instruction_encoding_flags * flags)
{
	jive_buffer_putstr(target, icls->mnemonic);
	jive_buffer_putstr(target, "\t%");
	jive_buffer_putstr(target, inputs[1]->base.name);
	jive_buffer_putstr(target, ", ");
	jive_buffer_putdisp(target, &immediates[0], inputs[0]);
}

static void
jive_i386_encode_regreg_readonly(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_immediate immediates[],
	jive_instruction_encoding_flags * flags)
{
	int r1 = inputs[0]->code;
	int r2 = inputs[1]->code;
	
	jive_buffer_putbyte(target, icls->code);
	jive_buffer_putbyte(target, 0xc0|r1|(r2<<3));
}

static void
jive_i386_encode_regreg(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_immediate immediates[],
	jive_instruction_encoding_flags * flags)
{
	JIVE_DEBUG_ASSERT(inputs[0] == outputs[0]);
	jive_i386_encode_regreg_readonly(icls, target, inputs, outputs, immediates, flags);
}

static void
jive_i386_asm_regreg(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_immediate immediates[],
	jive_instruction_encoding_flags * flags)
{
	jive_buffer_putstr(target, icls->mnemonic);
	jive_buffer_putstr(target, "\t%");
	jive_buffer_putstr(target, inputs[1]->base.name);
	jive_buffer_putstr(target, ", %");
	jive_buffer_putstr(target, inputs[0]->base.name);
}

static void
jive_i386_asm_imul(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_immediate immediates[],
	jive_instruction_encoding_flags * flags)
{
	jive_buffer_putstr(target, icls->mnemonic);
	jive_buffer_putstr(target, "\t%");
	jive_buffer_putstr(target, inputs[1]->base.name);
	
	jive_buffer_putstr(target, ", %");
	jive_buffer_putstr(target, inputs[0]->base.name);
}

static void
jive_i386_encode_imull(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_immediate immediates[],
	jive_instruction_encoding_flags * flags)
{
	int r1 = inputs[0]->code;
	int r2 = inputs[1]->code;

	jive_buffer_putbyte(target, icls->code);

	JIVE_DEBUG_ASSERT(r1 == outputs[1]->code);
	jive_buffer_putbyte(target, 0xe8|r2);	
}

static void
jive_i386_encode_mul_regreg(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_immediate immediates[],
	jive_instruction_encoding_flags * flags)
{
	int r1 = inputs[0]->code;
	int r2 = inputs[1]->code;
	
	JIVE_DEBUG_ASSERT(r1 == outputs[0]->code);
	
	jive_buffer_putbyte(target, 0x0f);
	jive_buffer_putbyte(target, 0xaf);
	jive_buffer_putbyte(target, 0xc0|r2|(r1<<3));
}

static void
jive_i386_asm_mul(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_immediate immediates[],
	jive_instruction_encoding_flags * flags)
{
	jive_buffer_putstr(target, icls->mnemonic);
	jive_buffer_putstr(target, "\t%");
	jive_buffer_putstr(target, inputs[1]->base.name);
}

static void
jive_i386_encode_mull(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_immediate immediates[],
	jive_instruction_encoding_flags * flags)
{
	int r1 = inputs[0]->code;
	int r2 = inputs[1]->code;

	JIVE_DEBUG_ASSERT(r1 == outputs[1]->code);

	jive_buffer_putbyte(target, icls->code);
	jive_buffer_putbyte(target, 0xe0|r2);
}

static void
jive_i386_asm_idiv_reg(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_immediate immediates[],
	jive_instruction_encoding_flags * flags)
{
	jive_buffer_putstr(target, icls->mnemonic);
	jive_buffer_putstr(target, "\t%");
	jive_buffer_putstr(target, inputs[3]->base.name);
}

static void
jive_i386_encode_idiv_reg(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_immediate immediates[],
	jive_instruction_encoding_flags * flags)
{
	int r3 = inputs[3]->code;

	jive_buffer_putbyte(target, 0xf7);
	jive_buffer_putbyte(target, 0xf8|r3);
}

static void
jive_i386_asm_div_reg(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_immediate immediates[],
	jive_instruction_encoding_flags * flags)
{
	jive_buffer_putstr(target, icls->mnemonic);
	jive_buffer_putstr(target, "\t%");
	jive_buffer_putstr(target, inputs[3]->base.name);
}

static void
jive_i386_encode_div_reg(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_immediate immediates[],
	jive_instruction_encoding_flags * flags)
{
	int r3 = inputs[3]->code;

	jive_buffer_putbyte(target, 0xf7);
	jive_buffer_putbyte(target, 0xf0|r3);
}

static void
jive_i386_encode_shift_regimm(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_immediate immediates[],
	jive_instruction_encoding_flags * flags)
{
	int r1 = inputs[0]->code;
	JIVE_DEBUG_ASSERT(r1 == outputs[0]->code);
	
	uint8_t count = (uint8_t) immediates[0].offset;
	
	if (count == 1) {
		jive_buffer_putbyte(target, 0xd1);
		jive_buffer_putbyte(target, icls->code | r1);
	} else {
		jive_buffer_putbyte(target, 0xc1);
		jive_buffer_putbyte(target, icls->code | r1);
		jive_buffer_putbyte(target, count);
	}
}

static void
jive_i386_asm_shift_regreg(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_immediate immediates[],
	jive_instruction_encoding_flags * flags)
{
	jive_buffer_putstr(target, icls->mnemonic);
	jive_buffer_putstr(target, "\t%cl, %");
	jive_buffer_putstr(target, inputs[0]->base.name);
}

static void
jive_i386_encode_shift_regreg(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_immediate immediates[],
	jive_instruction_encoding_flags * flags)
{
	int r1 = inputs[0]->code;
	JIVE_DEBUG_ASSERT(r1 == outputs[0]->code);
	
	jive_buffer_putbyte(target, 0xd3);
	jive_buffer_putbyte(target, icls->code | r1);
}

static void
jive_i386_encode_regimm_readonly(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_immediate immediates[],
	jive_instruction_encoding_flags * flags)
{
	int r1 = inputs[0]->code;
	int32_t immediate = immediates[0].offset;
	
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
jive_i386_encode_regimm(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_immediate immediates[],
	jive_instruction_encoding_flags * flags)
{
	JIVE_DEBUG_ASSERT(inputs[0] == outputs[0]);
	jive_i386_encode_regimm_readonly(icls, target, inputs, outputs, immediates, flags);
}

static void
jive_i386_asm_regimm(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_immediate immediates[],
	jive_instruction_encoding_flags * flags)
{
	jive_buffer_putstr(target, icls->mnemonic);
	jive_buffer_putstr(target, "\t$");
	jive_buffer_putimm(target, &immediates[0]);
	jive_buffer_putstr(target, ", %");
	jive_buffer_putstr(target, inputs[0]->base.name);
}

static void
jive_i386_encode_unaryreg(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_immediate immediates[],
	jive_instruction_encoding_flags * flags)
{
	int r1 = inputs[0]->code;
	
	JIVE_DEBUG_ASSERT(r1 == outputs[0]->code);
	
	jive_buffer_putbyte(target, 0xf7);
	jive_buffer_putbyte(target, icls->code|r1);
}

static void
jive_i386_asm_unaryreg(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_immediate immediates[],
	jive_instruction_encoding_flags * flags)
{
	jive_buffer_putstr(target, icls->mnemonic);
	jive_buffer_putstr(target, "\t%");
	jive_buffer_putstr(target, inputs[0]->base.name);
}

static void
jive_i386_encode_regmove(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_immediate immediates[],
	jive_instruction_encoding_flags * flags)
{
	int r1 = outputs[0]->code;
	int r2 = inputs[0]->code;
	
	jive_buffer_putbyte(target, icls->code);
	jive_buffer_putbyte(target, 0xc0|r1|(r2<<3));
}

static void
jive_i386_asm_regmove(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_immediate immediates[],
	jive_instruction_encoding_flags * flags)
{
	jive_buffer_putstr(target, icls->mnemonic);
	jive_buffer_putstr(target, "\t%");
	jive_buffer_putstr(target, inputs[0]->base.name);
	jive_buffer_putstr(target, ", %");
	jive_buffer_putstr(target, outputs[0]->base.name);
}

static void
jive_i386_encode_call(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_immediate immediates[],
	jive_instruction_encoding_flags * flags)
{
	uint32_t immediate = immediates[0].offset;
	
	jive_buffer_putbyte(target, icls->code);
	immediate = cpu_to_le32(immediate);
	jive_buffer_put(target, &immediate, 4);
}

static void
jive_i386_asm_call(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_immediate immediates[],
	jive_instruction_encoding_flags * flags)
{
	jive_buffer_putstr(target, icls->mnemonic);
	jive_buffer_putstr(target, "\t");
	jive_buffer_putimm(target, &immediates[0]);
}

static void
jive_i386_encode_call_reg(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_immediate immediates[],
	jive_instruction_encoding_flags * flags)
{
	int r = inputs[0]->code;
	
	jive_buffer_putbyte(target, 0xff);
	jive_buffer_putbyte(target, 0xd0 | r);
}

static void
jive_i386_asm_call_reg(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_immediate immediates[],
	jive_instruction_encoding_flags * flags)
{
	jive_buffer_putstr(target, icls->mnemonic);
	jive_buffer_putstr(target, "\t*%");
	jive_buffer_putstr(target, inputs[0]->base.name);
}

static void
jive_i386_asm_jump(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_immediate immediates[],
	jive_instruction_encoding_flags * flags)
{
	jive_buffer_putstr(target, icls->mnemonic);
	jive_buffer_putstr(target, "\t");
	jive_buffer_putimm(target, &immediates[0]);
}

static void
jive_i386_encode_jump(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_immediate immediates[],
	jive_instruction_encoding_flags * flags)
{
	/* test whether instruction needs to be changed to long form --
	this is the case when we either need a relocation entry, or we
	can statically determine that the jump is too far */
	if (jive_immediate_has_symbols(&immediates[0])) {
		*flags |= jive_instruction_encoding_flags_option0;
	} else {
		int64_t dist = immediates[0].offset - 2;
		if (dist > 0x80 || dist <= -0x80)
			*flags |= jive_instruction_encoding_flags_option0;
	}
	
	if ((*flags & jive_instruction_encoding_flags_option0) == 0) {
		/* short form */
		uint8_t dist = immediates[0].offset - 2;
		jive_buffer_putbyte(target, 0xeb);
		jive_buffer_putbyte(target, dist);
	} else {
		/* long form */
		uint32_t dist = cpu_to_le32(immediates[0].offset - 5);
		jive_buffer_putbyte(target, 0xe9);
		jive_buffer_put(target, &dist, sizeof(dist));
		/* FIXME: possibly add relocation entry */
	}
}

static void
jive_i386_encode_jump_conditional(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_immediate immediates[],
	jive_instruction_encoding_flags * flags)
{
	/* test whether instruction needs to be changed to long form --
	this is the case when we either need a relocation entry, or we
	can statically determine that the jump is too far */
	if (jive_immediate_has_symbols(&immediates[0])) {
		*flags |= jive_instruction_encoding_flags_option0;
	} else {
		int64_t dist = immediates[0].offset - 2;
		if (dist > 0x80 || dist <= -0x80)
			*flags |= jive_instruction_encoding_flags_option0;
	}
	
	if ((*flags & jive_instruction_encoding_flags_option0) == 0) {
		/* short form */
		uint8_t dist = immediates[0].offset - 2;
		jive_buffer_putbyte(target, 0x70 | icls->code);
		jive_buffer_putbyte(target, dist);
	} else {
		/* long form */
		uint32_t dist = cpu_to_le32(immediates[0].offset - 6);
		jive_buffer_putbyte(target, 0x0f);
		jive_buffer_putbyte(target, 0x80 | icls->code);
		jive_buffer_put(target, &dist, sizeof(dist));
		/* FIXME: possibly add relocation entry */
	}
}

static const jive_register_class * const intreg_param[] = {
	&jive_i386_regcls[jive_i386_gpr],
	&jive_i386_regcls[jive_i386_gpr],
	&jive_i386_regcls[jive_i386_gpr]
};

static const jive_register_class * const intflags_param[] = {
	&jive_i386_regcls[jive_i386_gpr],
	&jive_i386_regcls[jive_i386_flags]
};

const jive_instruction_class jive_i386_instructions[] = {
	[jive_i386_ret] = {
		.name = "ret",
		.mnemonic = "ret",
		.encode = jive_i386_encode_simple,
		.write_asm = jive_i386_asm_simple,
		.inregs = 0, .outregs = 0, .flags = jive_instruction_flags_none, .ninputs = 0, .noutputs = 0, .nimmediates = 0,
		.code = 0xc3
	},
	
	[jive_i386_int_load_imm] = {
		.name = "int_load_imm",
		.mnemonic = "movl",
		.encode = jive_i386_encode_int_load_imm,
		.write_asm = jive_i386_asm_int_load_imm,
		.inregs = 0, .outregs = intreg_param, .flags = jive_instruction_flags_none,
		.ninputs = 0, .noutputs = 1, .nimmediates = 1,
		.code = 0xb8
	},
	
	[jive_i386_int_load32_disp] = {
		.name = "int_load32_disp",
		.mnemonic = "movl",
		.encode = jive_i386_encode_loadstore32_disp,
		.write_asm = jive_i386_asm_load,
		.inregs = intreg_param, .outregs = intreg_param, .flags = jive_instruction_flags_none,
		.ninputs = 1, .noutputs = 1, .nimmediates = 1,
		.code = 0x8b
	},
	[jive_i386_int_store32_disp] = {
		.name = "int_store32_disp",
		.mnemonic = "movl",
		.encode = jive_i386_encode_loadstore32_disp,
		.write_asm = jive_i386_asm_store,
		.inregs = intreg_param, .outregs = 0, .flags = jive_instruction_flags_none,
		.ninputs = 2, .noutputs = 0, .nimmediates = 1,
		.code = 0x89
	},
	[jive_i386_int_add] = {
		.name = "int_add",
		.mnemonic = "addl",
		.encode = jive_i386_encode_regreg,
		.write_asm = jive_i386_asm_regreg,
		.inregs = intreg_param, .outregs = intflags_param, .flags = jive_instruction_write_input | jive_instruction_commutative,
		.ninputs = 2, .noutputs = 2, .nimmediates = 0,
		.code = 0x01
	},
	[jive_i386_int_sub] = {
		.name = "int_sub",
		.mnemonic = "subl",
		.encode = jive_i386_encode_regreg,
		.write_asm = jive_i386_asm_regreg,
		.inregs = intreg_param, .outregs = intflags_param, .flags = jive_instruction_write_input,
		.ninputs = 2, .noutputs = 2, .nimmediates = 0,
		.code = 0x29
	},
	[jive_i386_int_and] = {
		.name = "int_and",
		.mnemonic = "andl",
		.encode = jive_i386_encode_regreg,
		.write_asm = jive_i386_asm_regreg,
		.inregs = intreg_param, .outregs = intflags_param, .flags = jive_instruction_write_input | jive_instruction_commutative,
		.ninputs = 2, .noutputs = 2, .nimmediates = 0,
		.code = 0x21
	},
	[jive_i386_int_or] = {
		.name = "int_or",
		.mnemonic = "orl",
		.encode = jive_i386_encode_regreg,
		.write_asm = jive_i386_asm_regreg,
		.inregs = intreg_param, .outregs = intflags_param, .flags = jive_instruction_write_input | jive_instruction_commutative,
		.ninputs = 2, .noutputs = 2, .nimmediates = 0,
		.code = 0x09
	},
	[jive_i386_int_xor] = {
		.name = "int_xor",
		.mnemonic = "xorl",
		.encode = jive_i386_encode_regreg,
		.write_asm = jive_i386_asm_regreg,
		.inregs = intreg_param, .outregs = intflags_param, .flags = jive_instruction_write_input | jive_instruction_commutative,
		.ninputs = 2, .noutputs = 2, .nimmediates = 0,
		.code = 0x31
	},
	[jive_i386_int_mul] = {
		.name = "int_mul",
		.mnemonic = "imull",
		.encode = jive_i386_encode_mul_regreg,
		.write_asm = jive_i386_asm_regreg,
		.inregs = intreg_param,
		.outregs = intflags_param,
		.flags = jive_instruction_write_input | jive_instruction_commutative,
		.ninputs = 2, .noutputs = 2, .nimmediates = 0,
		.code = 0xc0af0f
	},
	[jive_i386_int_mul_expand_signed] = {
		.name = "int_mul_expand_signed",
		.mnemonic = "imull",
		.encode = jive_i386_encode_imull,
		.write_asm = jive_i386_asm_imul,
		.inregs = (const jive_register_class *[]){&jive_i386_regcls[jive_i386_gpr_eax],
			&jive_i386_regcls[jive_i386_gpr]},
		.outregs = (const jive_register_class *[]){&jive_i386_regcls[jive_i386_gpr_edx],
			&jive_i386_regcls[jive_i386_gpr_eax], &jive_i386_regcls[jive_i386_flags]},
		.flags = jive_instruction_commutative,
		.ninputs = 2, .noutputs = 3, .nimmediates = 0,
		.code = 0xf7
	},
	[jive_i386_int_mul_expand_unsigned] = {
		.name = "int_mul_expand_unsigned",
		.mnemonic = "mull",
		.encode = jive_i386_encode_mull,
		.write_asm = jive_i386_asm_mul,
		.inregs = (const jive_register_class *[]){&jive_i386_regcls[jive_i386_gpr_eax],
			&jive_i386_regcls[jive_i386_gpr]},
		.outregs = (const jive_register_class *[]){&jive_i386_regcls[jive_i386_gpr_edx],
			&jive_i386_regcls[jive_i386_gpr_eax], &jive_i386_regcls[jive_i386_flags]},
		.flags = jive_instruction_commutative,
		.ninputs = 2, .noutputs = 3, .nimmediates = 0,
		.code = 0xf7
	},
	[jive_i386_int_sdiv] = {
		.name = "int_sdiv",
		.mnemonic = "idivl",
		.encode = jive_i386_encode_idiv_reg,
		.write_asm = jive_i386_asm_idiv_reg,
		.inregs = (const jive_register_class *[]){&jive_i386_regcls[jive_i386_gpr_edx],
			&jive_i386_regcls[jive_i386_gpr_eax], &jive_i386_regcls[jive_i386_gpr]},
		.outregs = (const jive_register_class *[]){&jive_i386_regcls[jive_i386_gpr_edx],
			&jive_i386_regcls[jive_i386_gpr_eax], &jive_i386_regcls[jive_i386_flags]}, 
		.flags = jive_instruction_flags_none,
		.ninputs = 3, .noutputs = 3, .nimmediates = 0,
		.code = 0xf8f7 
	},
	[jive_i386_int_udiv] = {
		.name = "int_udiv",
		.mnemonic = "divl",
		.encode = jive_i386_encode_div_reg,
		.write_asm = jive_i386_asm_div_reg,
		.inregs = (const jive_register_class *[]){&jive_i386_regcls[jive_i386_gpr_edx],
			&jive_i386_regcls[jive_i386_gpr_eax], &jive_i386_regcls[jive_i386_gpr]},
		.outregs = (const jive_register_class *[]){&jive_i386_regcls[jive_i386_gpr_edx],
			&jive_i386_regcls[jive_i386_gpr_eax], &jive_i386_regcls[jive_i386_flags]},
		.flags = jive_instruction_flags_none,
		.ninputs = 3, .noutputs = 3, .nimmediates = 0,
		.code = 0xf0f7
	},
	
	/* for the immediate instructions, code consists of normal_code | (eax_code << 8),
	see instruction coding function for explanation */
	[jive_i386_int_add_immediate] = {
		.name = "int_add_immediate",
		.mnemonic = "addl",
		.encode = jive_i386_encode_regimm,
		.write_asm = jive_i386_asm_regimm,
		.inregs = intreg_param, .outregs = intflags_param, .flags = jive_instruction_write_input,
		.ninputs = 1, .noutputs = 2, .nimmediates = 1,
		.code = 0xc0 | (0x05 << 8)
	},
	[jive_i386_int_sub_immediate] = {
		.name = "int_sub_immediate",
		.mnemonic = "subl",
		.encode = jive_i386_encode_regimm,
		.write_asm = jive_i386_asm_regimm,
		.inregs = intreg_param, .outregs = intflags_param, .flags = jive_instruction_write_input,
		.ninputs = 1, .noutputs = 2, .nimmediates = 1,
		.code = 0xe8 | (0x2d << 8)
	},
	[jive_i386_int_and_immediate] = {
		.name = "int_and_immediate",
		.mnemonic = "andl",
		.encode = jive_i386_encode_regimm,
		.write_asm = jive_i386_asm_regimm,
		.inregs = intreg_param, .outregs = intflags_param, .flags = jive_instruction_write_input,
		.ninputs = 1, .noutputs = 2, .nimmediates = 1,
		.code = 0xe0 | (0x25 << 8)
	},
	[jive_i386_int_or_immediate] = {
		.name = "int_or_immediate",
		.mnemonic = "orl",
		.encode = jive_i386_encode_regimm,
		.write_asm = jive_i386_asm_regimm,
		.inregs = intreg_param, .outregs = intflags_param, .flags = jive_instruction_write_input,
		.ninputs = 1, .noutputs = 2, .nimmediates = 1,
		.code = 0xc8 | (0x0d << 8)
	},
	[jive_i386_int_xor_immediate] = {
		.name = "int_xor_immediate",
		.mnemonic = "xorl",
		.encode = jive_i386_encode_regimm,
		.write_asm = jive_i386_asm_regimm,
		.inregs = intreg_param, .outregs = intflags_param, .flags = jive_instruction_write_input,
		.ninputs = 1, .noutputs = 2, .nimmediates = 1,
		.code = 0xc0 | (0x3d << 8)
	},
	[jive_i386_int_neg] = {
		.name = "int_neg",
		.mnemonic = "negl",
		.encode = jive_i386_encode_unaryreg,
		.write_asm = jive_i386_asm_unaryreg,
		.inregs = intreg_param, .outregs = intflags_param, .flags = jive_instruction_write_input,
		.ninputs = 1, .noutputs = 2, .nimmediates = 0,
		.code = 0xd8
	},
	[jive_i386_int_not] = {
		.name = "int_not",
		.mnemonic = "notl",
		.encode = jive_i386_encode_unaryreg,
		.write_asm = jive_i386_asm_unaryreg,
		.inregs = intreg_param, .outregs = intflags_param, .flags = jive_instruction_write_input,
		.ninputs = 1, .noutputs = 2, .nimmediates = 0,
		.code = 0xd0
	},
	[jive_i386_int_shr] = {
		.name = "int_shr",
		.mnemonic = "shrl",
		.encode = jive_i386_encode_shift_regreg,
		.write_asm = jive_i386_asm_shift_regreg,
		.inregs = (const jive_register_class *[]){&jive_i386_regcls[jive_i386_gpr],
			&jive_i386_regcls[jive_i386_gpr_ecx]},
		.outregs = intflags_param,
		.flags = jive_instruction_write_input,
		.ninputs = 2, .noutputs = 2, .nimmediates = 0,
		.code = 0xe8
	},
	[jive_i386_int_shr_immediate] = {
		.name = "int_shr_immediate",
		.mnemonic = "shrl",
		.encode = jive_i386_encode_shift_regimm,
		.write_asm = jive_i386_asm_regimm,
		.inregs = (const jive_register_class *[]){&jive_i386_regcls[jive_i386_gpr]},
		.outregs = intflags_param,
		.flags = jive_instruction_write_input,
		.ninputs = 1, .noutputs = 2, .nimmediates = 0,
		.code = 0xe8
	},
	[jive_i386_int_shl] = {
		.name = "int_shl",
		.mnemonic = "shll",
		.encode = jive_i386_encode_shift_regreg,
		.write_asm = jive_i386_asm_shift_regreg,
		.inregs = (const jive_register_class *[]){&jive_i386_regcls[jive_i386_gpr],
			&jive_i386_regcls[jive_i386_gpr_ecx]},
		.outregs = intflags_param,
		.flags = jive_instruction_write_input,
		.ninputs = 2, .noutputs = 2, .nimmediates = 0,
		.code = 0xe0
	},
	[jive_i386_int_shl_immediate] = {
		.name = "int_shl_immediate",
		.mnemonic = "shll",
		.encode = jive_i386_encode_shift_regimm,
		.write_asm = jive_i386_asm_regimm,
		.inregs = (const jive_register_class *[]){&jive_i386_regcls[jive_i386_gpr]},
		.outregs = intflags_param,
		.flags = jive_instruction_write_input,
		.ninputs = 1, .noutputs = 2, .nimmediates = 0,
		.code = 0xe0
	},
	[jive_i386_int_ashr] = {
		.name = "int_ashr",
		.mnemonic = "sarl",
		.encode = jive_i386_encode_shift_regreg,
		.write_asm = jive_i386_asm_shift_regreg,
		.inregs = (const jive_register_class *[]){&jive_i386_regcls[jive_i386_gpr],
			&jive_i386_regcls[jive_i386_gpr_ecx]},
		.outregs = intflags_param,
		.flags = jive_instruction_write_input,
		.ninputs = 2, .noutputs = 2, .nimmediates = 0,
		.code = 0xf8
	},
	[jive_i386_int_ashr_immediate] = {
		.name = "int_ashr_immediate",
		.mnemonic = "sarl",
		.encode = jive_i386_encode_shift_regimm,
		.write_asm = jive_i386_asm_regimm,
		.inregs = (const jive_register_class *[]){&jive_i386_regcls[jive_i386_gpr]},
		.outregs = intflags_param,
		.flags = jive_instruction_write_input,
		.ninputs = 1, .noutputs = 2, .nimmediates = 0,
		.code = 0xf8
	},
	
	[jive_i386_int_transfer] = {
		.name = "int_transfer",
		.mnemonic = "movl",
		.encode = jive_i386_encode_regmove,
		.write_asm = jive_i386_asm_regmove,
		.inregs = intreg_param, .outregs = intreg_param,
		.ninputs = 1, .noutputs = 1, .nimmediates = 0,
		.code = 0x89
	},
	[jive_i386_call] = {
		.name = "call",
		.mnemonic = "call",
		.encode = jive_i386_encode_call,
		.write_asm = jive_i386_asm_call,
		.inregs = NULL, .outregs = NULL,
		.ninputs = 0, .noutputs = 0, .nimmediates = 1,
		.code = 0xe8
	},
	[jive_i386_call_reg] = {
		.name = "call_reg",
		.mnemonic = "call_reg",
		.encode = jive_i386_encode_call_reg,
		.write_asm = jive_i386_asm_call_reg,
		.inregs = intreg_param, .outregs = NULL,
		.ninputs = 1, .noutputs = 0, .nimmediates = 0,
		.code = 0xff
	},
	[jive_i386_int_cmp] = {
		.name = "int_cmp",
		.mnemonic = "cmpl",
		.encode = jive_i386_encode_regreg_readonly,
		.write_asm = jive_i386_asm_regreg,
		.inregs = intreg_param,
		.outregs = (const jive_register_class *[]){&jive_i386_regcls[jive_i386_flags]},
		.flags = jive_instruction_flags_none,
		.ninputs = 2, .noutputs = 1, .nimmediates = 0,
		.code = 0x3b
	},
	[jive_i386_int_cmp_immediate] = {
		.name = "int_cmp_immediate",
		.mnemonic = "cmpl",
		.encode = jive_i386_encode_regimm_readonly,
		.write_asm = jive_i386_asm_regimm,
		.inregs = intreg_param,
		.outregs = (const jive_register_class *[]){&jive_i386_regcls[jive_i386_flags]},
		.flags = jive_instruction_jump | jive_instruction_jump_relative | jive_instruction_jump_conditional_invertible,
		.ninputs = 1, .noutputs = 1, .nimmediates = 1,
		.code = 0x81 
	},
	[jive_i386_int_jump_sless] = {
		.name = "int_jump_sless",
		.mnemonic = "jl",
		.encode = jive_i386_encode_jump_conditional,
		.write_asm = jive_i386_asm_jump,
		.inregs = (const jive_register_class *[]){&jive_i386_regcls[jive_i386_flags]},
		.outregs = NULL,
		.flags = jive_instruction_jump | jive_instruction_jump_relative | jive_instruction_jump_conditional_invertible,
		.ninputs = 1, .noutputs = 0, .nimmediates = 1,
		.code = 0xc,
		.inverse_jump = &jive_i386_instructions[jive_i386_int_jump_sgreatereq]
	},
	[jive_i386_int_jump_uless] = {
		.name = "int_jump_uless",
		.mnemonic = "jb",
		.encode = jive_i386_encode_jump_conditional,
		.write_asm = jive_i386_asm_jump,
		.inregs = (const jive_register_class *[]){&jive_i386_regcls[jive_i386_flags]},
		.outregs = NULL,
		.flags = jive_instruction_jump | jive_instruction_jump_relative | jive_instruction_jump_conditional_invertible,
		.ninputs = 1, .noutputs = 0, .nimmediates = 1,
		.code = 0x2,
		.inverse_jump = &jive_i386_instructions[jive_i386_int_jump_ugreatereq]
	},
	[jive_i386_int_jump_slesseq] = {
		.name = "int_jump_slesseq",
		.mnemonic = "jle",
		.encode = jive_i386_encode_jump_conditional,
		.write_asm = jive_i386_asm_jump,
		.inregs = (const jive_register_class *[]){&jive_i386_regcls[jive_i386_flags]},
		.outregs = NULL,
		.flags = jive_instruction_jump | jive_instruction_jump_relative | jive_instruction_jump_conditional_invertible,
		.ninputs = 1, .noutputs = 0, .nimmediates = 1,
		.code = 0xe,
		.inverse_jump = &jive_i386_instructions[jive_i386_int_jump_sgreater]
	},
	[jive_i386_int_jump_ulesseq] = {
		.name = "int_jump_ulesseq",
		.mnemonic = "jbe",
		.encode = jive_i386_encode_jump_conditional,
		.write_asm = jive_i386_asm_jump,
		.inregs = (const jive_register_class *[]){&jive_i386_regcls[jive_i386_flags]},
		.outregs = NULL,
		.flags = jive_instruction_jump | jive_instruction_jump_relative | jive_instruction_jump_conditional_invertible,
		.ninputs = 1, .noutputs = 0, .nimmediates = 1,
		.code = 0x6,
		.inverse_jump = &jive_i386_instructions[jive_i386_int_jump_ugreater]
	},
	[jive_i386_int_jump_equal] = {
		.name = "int_jump_equal",
		.mnemonic = "je",
		.encode = jive_i386_encode_jump_conditional,
		.write_asm = jive_i386_asm_jump,
		.inregs = (const jive_register_class *[]){&jive_i386_regcls[jive_i386_flags]},
		.outregs = NULL,
		.flags = jive_instruction_jump | jive_instruction_jump_relative | jive_instruction_jump_conditional_invertible,
		.ninputs = 1, .noutputs = 0, .nimmediates = 1,
		.code = 0x4,
		.inverse_jump = &jive_i386_instructions[jive_i386_int_jump_notequal]
	},
	[jive_i386_int_jump_notequal] = {
		.name = "int_jump_notequal",
		.mnemonic = "jne",
		.encode = jive_i386_encode_jump_conditional,
		.write_asm = jive_i386_asm_jump,
		.inregs = (const jive_register_class *[]){&jive_i386_regcls[jive_i386_flags]},
		.outregs = NULL,
		.flags = jive_instruction_jump | jive_instruction_jump_relative | jive_instruction_jump_conditional_invertible,
		.ninputs = 1, .noutputs = 0, .nimmediates = 1,
		.code = 0x5,
		.inverse_jump = &jive_i386_instructions[jive_i386_int_jump_equal]
	},
	[jive_i386_int_jump_sgreater] = {
		.name = "int_jump_sgreater",
		.mnemonic = "jg",
		.encode = jive_i386_encode_jump_conditional,
		.write_asm = jive_i386_asm_jump,
		.inregs = (const jive_register_class *[]){&jive_i386_regcls[jive_i386_flags]},
		.outregs = NULL,
		.flags = jive_instruction_jump | jive_instruction_jump_relative | jive_instruction_jump_conditional_invertible,
		.ninputs = 1, .noutputs = 0, .nimmediates = 1,
		.code = 0xf,
		.inverse_jump = &jive_i386_instructions[jive_i386_int_jump_slesseq]
	},
	[jive_i386_int_jump_ugreater] = {
		.name = "int_jump_ugreater",
		.mnemonic = "ja",
		.encode = jive_i386_encode_jump_conditional,
		.write_asm = jive_i386_asm_jump,
		.inregs = (const jive_register_class *[]){&jive_i386_regcls[jive_i386_flags]},
		.outregs = NULL,
		.flags = jive_instruction_jump | jive_instruction_jump_relative | jive_instruction_jump_conditional_invertible,
		.ninputs = 1, .noutputs = 0, .nimmediates = 1,
		.code = 0x7,
		.inverse_jump = &jive_i386_instructions[jive_i386_int_jump_ulesseq]
	},
	[jive_i386_int_jump_sgreatereq] = {
		.name = "int_jump_sgreatereq",
		.mnemonic = "jge",
		.encode = jive_i386_encode_jump_conditional,
		.write_asm = jive_i386_asm_jump,
		.inregs = (const jive_register_class *[]){&jive_i386_regcls[jive_i386_flags]},
		.outregs = NULL,
		.flags = jive_instruction_jump | jive_instruction_jump_relative | jive_instruction_jump_conditional_invertible,
		.ninputs = 1, .noutputs = 0, .nimmediates = 1,
		.code = 0xd,
		.inverse_jump = &jive_i386_instructions[jive_i386_int_jump_sless]
	},
	[jive_i386_int_jump_ugreatereq] = {
		.name = "int_jump_ugreatereq",
		.mnemonic = "jae",
		.encode = jive_i386_encode_jump_conditional,
		.write_asm = jive_i386_asm_jump,
		.inregs = (const jive_register_class *[]){&jive_i386_regcls[jive_i386_flags]},
		.outregs = NULL,
		.flags = jive_instruction_jump | jive_instruction_jump_relative | jive_instruction_jump_conditional_invertible,
		.ninputs = 1, .noutputs = 0, .nimmediates = 1,
		.code = 0x3,
		.inverse_jump = &jive_i386_instructions[jive_i386_int_jump_uless]
	},
	[jive_i386_jump] = {
		.name = "jump",
		.mnemonic = "jmp",
		.encode = jive_i386_encode_jump,
		.write_asm = jive_i386_asm_jump,
		.inregs = NULL,
		.outregs = NULL,
		.flags = jive_instruction_jump_relative,
		.ninputs = 0, .noutputs = 0, .nimmediates = 1,
		.code = 0xeb,
		.inverse_jump = NULL
	},
};

static const jive_instructionset_class jive_i386_instructionset_class = {
	.create_xfer = jive_i386_create_xfer,
};

const struct jive_instructionset jive_i386_instructionset = {
	.class_ = &jive_i386_instructionset_class,
	.jump_instruction_class = &jive_i386_instructions[jive_i386_jump],
	.reg_classifier = &jive_i386_reg_classifier
};
