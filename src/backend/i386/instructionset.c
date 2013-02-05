/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/compilate.h>
#include <jive/arch/instructionset.h>
#include <jive/backend/i386/classifier.h>
#include <jive/backend/i386/registerset.h>
#include <jive/backend/i386/relocation.h>
#include <jive/backend/i386/instructionset.h>
#include <jive/backend/i386/machine.h>
#include <jive/serialization/instrcls-registry.h>
#include <jive/util/buffer.h>

#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>

static inline uint32_t
cpu_to_le32(uint32_t value)
{
	/* FIXME: endianness */
	return value;
}

static void
jive_buffer_putimm(jive_buffer * target, const jive_asmgen_imm * imm)
{
	bool empty = true;
	if (imm->value) {
		char tmp[80];
		snprintf(tmp, sizeof(tmp), "%" PRId64, imm->value);
		jive_buffer_putstr(target, tmp);
		empty = false;
	}
	if (imm->add_symbol) {
		if (!empty)
			jive_buffer_putstr(target, "+");
		jive_buffer_putstr(target, imm->add_symbol);
		empty = false;
	}
	if (imm->sub_symbol) {
		jive_buffer_putstr(target, "-");
		jive_buffer_putstr(target, imm->sub_symbol);
		empty = false;
	}
	if (empty) {
		jive_buffer_putstr(target, "0");
	}
}

/* test whether the given immediate (plus offset) must be assumed to be
outside the [-0x80..0x80) range, thus forcing to do an alternate instruction
encoding; the encoding flags are also checked and possibly updated, to
ensure that instruction encoding ultimately reaches a fixed point */
static inline bool
jive_i386_check_long_form(
	const jive_codegen_imm * imm,
	jive_instruction_encoding_flags * flags,
	jive_immediate_int offset)
{
	bool need_long_form;
	
	switch (imm->info) {
		case jive_codegen_imm_info_dynamic_known:
		case jive_codegen_imm_info_static_known: {
			int64_t dist = imm->value + offset;
			need_long_form = (dist >= 0x80) || (dist < -0x80);
			break;
		}
		case jive_codegen_imm_info_static_unknown: {
			need_long_form = true;
			break;
		}
		case jive_codegen_imm_info_dynamic_unknown: {
			need_long_form = false;
			break;
		}
	}
	
	if ( (*flags & jive_instruction_encoding_flags_option0) != 0) {
		need_long_form = true;
	} else if (need_long_form) {
		*flags |= jive_instruction_encoding_flags_option0;
	}
	
	return need_long_form;
}

static void
jive_i386_encode_imm8(
	const jive_codegen_imm * imm,
	jive_immediate_int offset,
	size_t coded_size_so_far,
	jive_section * target)
{
	jive_relocation_type reltype =
		imm->pc_relative ?
		JIVE_R_386_PC8 :
		JIVE_R_386_8;
	
	uint8_t value = imm->value + offset;
	if (imm->pc_relative)
		value -= coded_size_so_far;
	
	switch (imm->info) {
		case jive_codegen_imm_info_dynamic_known:
		case jive_codegen_imm_info_static_known: {
			jive_section_putbyte(target, value);
			break;
		}
		case jive_codegen_imm_info_dynamic_unknown: {
			jive_section_putbyte(target, 0);
			break;
		}
		case jive_codegen_imm_info_static_unknown: {
			jive_section_put_reloc(target, &value, 1,
				reltype, imm->symref, 0);
			break;
		}
	}
}

static void
jive_i386_encode_imm32(
	const jive_codegen_imm * imm,
	jive_immediate_int offset,
	size_t coded_size_so_far,
	jive_section * target)
{
	jive_relocation_type reltype =
		imm->pc_relative ?
		JIVE_R_386_PC32 :
		JIVE_R_386_32;
	
	uint32_t value = imm->value + offset;
	if (imm->pc_relative)
		value -= coded_size_so_far;
	value = cpu_to_le32(value);
	
	switch (imm->info) {
		case jive_codegen_imm_info_dynamic_known:
		case jive_codegen_imm_info_static_known: {
			jive_section_put(target, &value, sizeof(value));
			break;
		}
		case jive_codegen_imm_info_dynamic_unknown: {
			value = 0;
			jive_section_put(target, &value, sizeof(value));
			break;
		}
		case jive_codegen_imm_info_static_unknown: {
			jive_section_put_reloc(target, &value, sizeof(value),
				reltype, imm->symref, 0);
			break;
		}
	}
}

static void
jive_buffer_putdisp(jive_buffer * target, const jive_asmgen_imm * disp, const jive_register_name * reg)
{
	jive_buffer_putimm(target, disp);
	jive_buffer_putstr(target, "(%");
	jive_buffer_putstr(target, reg->base.name);
	jive_buffer_putstr(target, ")");
}

static void
jive_i386_encode_simple(const jive_instruction_class * icls,
	jive_section * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	jive_section_putbyte(target, icls->code);
}

static void
jive_i386_asm_simple(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_asmgen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	jive_buffer_putstr(target, icls->mnemonic);
}

static void
jive_i386_encode_int_load_imm(const jive_instruction_class * icls,
	jive_section * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	int reg = outputs[0]->code;
	jive_section_putbyte(target, icls->code|reg);
	jive_i386_encode_imm32(&immediates[0], 0, 1, target);
}

static void
jive_i386_asm_int_load_imm(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_asmgen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	jive_buffer_putstr(target, icls->mnemonic);
	jive_buffer_putstr(target, "\t$");
	jive_buffer_putimm(target, &immediates[0]);
	jive_buffer_putstr(target, ", %");
	jive_buffer_putstr(target, outputs[0]->base.name);
}

static inline void
jive_i386_r2i(
	const jive_register_name * r1,
	const jive_register_name * r2,
	const jive_codegen_imm * imm,
	size_t coded_size_so_far,
	jive_instruction_encoding_flags * flags,
	jive_section * target)
{
	bool need_long_form = jive_i386_check_long_form(imm, flags, 0);
	
	int regcode = (r1->code) | (r2->code<<3);
	
	/* special treatment for load/store through ebp: always encode displacement parameter */
	bool code_displacement =
		(r1->code == 5) ||
		need_long_form ||
		imm->value != 0 ||
		imm->info != jive_codegen_imm_info_static_known;
	
	if (code_displacement) {
		if (need_long_form)
			regcode |= 0x80;
		else
			regcode |= 0x40;
	}
	
	jive_section_putbyte(target, regcode);
	coded_size_so_far ++;
	if (r1->code == 4) {
		/* esp special treatment */
		jive_section_putbyte(target, 0x24);
		coded_size_so_far ++;
	}
	
	if (code_displacement) {
		if (need_long_form) {
			jive_i386_encode_imm32(imm, 0, coded_size_so_far, target);
		} else {
			jive_i386_encode_imm8(imm, 0, coded_size_so_far, target);
		}
	}
}

static void
jive_i386_encode_loadstore32_disp(const jive_instruction_class * icls,
	jive_section * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	const jive_register_name * r1 = inputs[0], * r2;
	if (icls->code == 0x89)
		r2 = inputs[1];
	else
		r2 = outputs[0];
	
	jive_section_putbyte(target, icls->code);
	
	jive_i386_r2i(r1, r2, &immediates[0], 1, flags, target);
}

static void
jive_i386_asm_load_disp(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_asmgen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	jive_buffer_putstr(target, icls->mnemonic);
	jive_buffer_putstr(target, "\t");
	jive_buffer_putdisp(target, &immediates[0], inputs[0]);
	jive_buffer_putstr(target, ", %");
	jive_buffer_putstr(target, outputs[0]->base.name);
}

static void
jive_i386_asm_load_abs(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_asmgen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	jive_buffer_putstr(target, icls->mnemonic);
	jive_buffer_putstr(target, "\t");
	jive_buffer_putimm(target, &immediates[0]);
	jive_buffer_putstr(target, ", %");
	jive_buffer_putstr(target, outputs[0]->base.name);
}

static void
jive_i386_asm_store(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_asmgen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	jive_buffer_putstr(target, icls->mnemonic);
	jive_buffer_putstr(target, "\t%");
	jive_buffer_putstr(target, inputs[1]->base.name);
	jive_buffer_putstr(target, ", ");
	jive_buffer_putdisp(target, &immediates[0], inputs[0]);
}

static void
jive_i386_encode_regreg(const jive_instruction_class * icls,
	jive_section * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	JIVE_DEBUG_ASSERT(inputs[0] == outputs[0]);
	
	int r1 = inputs[0]->code;
	int r2 = inputs[1]->code;
	
	jive_section_putbyte(target, icls->code);
	jive_section_putbyte(target, 0xc0|r1|(r2<<3));
}

static void
jive_i386_asm_regreg(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_asmgen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	jive_buffer_putstr(target, icls->mnemonic);
	jive_buffer_putstr(target, "\t%");
	jive_buffer_putstr(target, inputs[1]->base.name);
	jive_buffer_putstr(target, ", %");
	jive_buffer_putstr(target, inputs[0]->base.name);
}

static void
jive_i386_encode_cmp_regreg(const jive_instruction_class * icls,
	jive_section * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	int r1 = inputs[1]->code;
	int r2 = inputs[0]->code;
	
	jive_section_putbyte(target, icls->code);
	jive_section_putbyte(target, 0xc0|r1|(r2<<3));
}

static void
jive_i386_asm_imul(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_asmgen_imm immediates[],
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
	jive_section * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	int r1 = inputs[0]->code;
	int r2 = inputs[1]->code;

	jive_section_putbyte(target, icls->code);

	JIVE_DEBUG_ASSERT(r1 == outputs[1]->code);
	jive_section_putbyte(target, 0xe8|r2);	
}

static void
jive_i386_encode_mul_regreg(const jive_instruction_class * icls,
	jive_section * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	int r1 = inputs[0]->code;
	int r2 = inputs[1]->code;
	
	JIVE_DEBUG_ASSERT(r1 == outputs[0]->code);
	
	jive_section_putbyte(target, 0x0f);
	jive_section_putbyte(target, 0xaf);
	jive_section_putbyte(target, 0xc0|r2|(r1<<3));
}

static void
jive_i386_asm_mul(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_asmgen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	jive_buffer_putstr(target, icls->mnemonic);
	jive_buffer_putstr(target, "\t%");
	jive_buffer_putstr(target, inputs[1]->base.name);
}

static void
jive_i386_encode_mull(const jive_instruction_class * icls,
	jive_section * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	int r1 = inputs[0]->code;
	int r2 = inputs[1]->code;

	JIVE_DEBUG_ASSERT(r1 == outputs[1]->code);

	jive_section_putbyte(target, icls->code);
	jive_section_putbyte(target, 0xe0|r2);
}

static void
jive_i386_asm_div_reg(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_asmgen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	jive_buffer_putstr(target, icls->mnemonic);
	jive_buffer_putstr(target, "\t%");
	jive_buffer_putstr(target, inputs[2]->base.name);
}

static void
jive_i386_encode_div_reg(const jive_instruction_class * icls,
	jive_section * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	int r = inputs[2]->code;

	jive_section_putbyte(target, 0xf7);
	jive_section_putbyte(target, icls->code | r);
}

static void
jive_i386_encode_shift_regimm(const jive_instruction_class * icls,
	jive_section * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	int r1 = inputs[0]->code;
	JIVE_DEBUG_ASSERT(r1 == outputs[0]->code);
	
	bool code_constant_one =
		immediates[0].info == jive_codegen_imm_info_static_known &&
		immediates[0].value == 1 &&
		immediates[0].symref.type == jive_symref_type_none;
	
	if (code_constant_one) {
		jive_section_putbyte(target, 0xd1);
		jive_section_putbyte(target, icls->code | r1);
	} else {
		jive_section_putbyte(target, 0xc1);
		jive_section_putbyte(target, icls->code | r1);
		jive_i386_encode_imm8(&immediates[0], 0, 2, target);
	}
}

static void
jive_i386_asm_shift_regreg(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_asmgen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	jive_buffer_putstr(target, icls->mnemonic);
	jive_buffer_putstr(target, "\t%cl, %");
	jive_buffer_putstr(target, inputs[0]->base.name);
}

static void
jive_i386_encode_shift_regreg(const jive_instruction_class * icls,
	jive_section * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	int r1 = inputs[0]->code;
	JIVE_DEBUG_ASSERT(r1 == outputs[0]->code);
	
	jive_section_putbyte(target, 0xd3);
	jive_section_putbyte(target, icls->code | r1);
}

static void
jive_i386_encode_regimm_readonly(const jive_instruction_class * icls,
	jive_section * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	int r1 = inputs[0]->code;
	
	bool need_long_form = jive_i386_check_long_form(&immediates[0], flags, 0);
	size_t coded_size_so_far = 0;
	
	char prefix = need_long_form ? 0x81 : 0x83;
	
	if (r1 == 0 && need_long_form) {
		char opcode = icls->code >> 8;
		jive_section_putbyte(target, opcode);
		coded_size_so_far ++;
	} else {
		char opcode = icls->code & 255;
		jive_section_putbyte(target, prefix);
		jive_section_putbyte(target, opcode | r1);
		coded_size_so_far += 2;
	}
	
	if (need_long_form)
		jive_i386_encode_imm32(&immediates[0], 0, coded_size_so_far, target);
	else
		jive_i386_encode_imm8(&immediates[0], 0, coded_size_so_far, target);
}

static void
jive_i386_encode_regimm(const jive_instruction_class * icls,
	jive_section * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_codegen_imm immediates[],
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
	const jive_asmgen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	jive_buffer_putstr(target, icls->mnemonic);
	jive_buffer_putstr(target, "\t$");
	jive_buffer_putimm(target, &immediates[0]);
	jive_buffer_putstr(target, ", %");
	jive_buffer_putstr(target, inputs[0]->base.name);
}

static void
jive_i386_encode_mul_regimm(const jive_instruction_class * icls,
	jive_section * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	int r1 = inputs[0]->code;
	int r2 = outputs[0]->code;
	
	bool need_long_form = jive_i386_check_long_form(&immediates[0], flags, 0);
	
	char regcode = 0xc0 | (r2 << 3) | (r1 << 0);
	
	if (need_long_form) {
		jive_section_putbyte(target, 0x69);
		jive_section_putbyte(target, regcode);
		jive_i386_encode_imm32(&immediates[0], 0, 2, target);
	} else {
		jive_section_putbyte(target, 0x6b);
		jive_section_putbyte(target, regcode);
		jive_i386_encode_imm8(&immediates[0], 0, 2, target);
	}
}

static void
jive_i386_asm_mul_regimm(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_asmgen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	jive_buffer_putstr(target, icls->mnemonic);
	jive_buffer_putstr(target, "\t$");
	jive_buffer_putimm(target, &immediates[0]);
	jive_buffer_putstr(target, ", %");
	jive_buffer_putstr(target, inputs[0]->base.name);
	jive_buffer_putstr(target, ", %");
	jive_buffer_putstr(target, outputs[0]->base.name);
}

static void
jive_i386_encode_unaryreg(const jive_instruction_class * icls,
	jive_section * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	int r1 = inputs[0]->code;
	
	JIVE_DEBUG_ASSERT(r1 == outputs[0]->code);
	
	jive_section_putbyte(target, 0xf7);
	jive_section_putbyte(target, icls->code|r1);
}

static void
jive_i386_asm_unaryreg(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_asmgen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	jive_buffer_putstr(target, icls->mnemonic);
	jive_buffer_putstr(target, "\t%");
	jive_buffer_putstr(target, inputs[0]->base.name);
}

static void
jive_i386_encode_regmove(const jive_instruction_class * icls,
	jive_section * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	int r1 = outputs[0]->code;
	int r2 = inputs[0]->code;
	
	jive_section_putbyte(target, icls->code);
	jive_section_putbyte(target, 0xc0|r1|(r2<<3));
}

static void
jive_i386_asm_regmove(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_asmgen_imm immediates[],
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
	jive_section * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	jive_section_putbyte(target, icls->code);
	jive_i386_encode_imm32(&immediates[0], 0, 1, target);
}

static void
jive_i386_asm_call(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_asmgen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	jive_buffer_putstr(target, icls->mnemonic);
	jive_buffer_putstr(target, "\t");
	jive_buffer_putimm(target, &immediates[0]);
}

static void
jive_i386_encode_call_reg(const jive_instruction_class * icls,
	jive_section * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	int r = inputs[0]->code;
	
	jive_section_putbyte(target, 0xff);
	jive_section_putbyte(target, 0xd0 | r);
}

static void
jive_i386_asm_call_reg(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_asmgen_imm immediates[],
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
	const jive_asmgen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	jive_buffer_putstr(target, icls->mnemonic);
	jive_buffer_putstr(target, "\t");
	jive_buffer_putimm(target, &immediates[0]);
}

static void
jive_i386_encode_jump(const jive_instruction_class * icls,
	jive_section * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	bool need_long_form = jive_i386_check_long_form(&immediates[0], flags, -2);
	
	if (!need_long_form) {
		jive_section_putbyte(target, 0xeb);
		jive_i386_encode_imm8(&immediates[0], -2, 1, target);
	} else {
		jive_section_putbyte(target, 0xe9);
		jive_i386_encode_imm32(&immediates[0], -5, 1, target);
	}
}

static void
jive_i386_encode_jump_conditional(const jive_instruction_class * icls,
	jive_section * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	bool need_long_form = jive_i386_check_long_form(&immediates[0], flags, -2);
	
	if (!need_long_form) {
		jive_section_putbyte(target, 0x70 | icls->code);
		jive_i386_encode_imm8(&immediates[0], -2, 1, target);
	} else {
		jive_section_putbyte(target, 0x0f);
		jive_section_putbyte(target, 0x80 | icls->code);
		jive_i386_encode_imm32(&immediates[0], -6, 2, target);
	}
}

static void
jive_i386_encode_loadstoresse_disp(const jive_instruction_class * icls,
	jive_section * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	const jive_register_name * r1 = inputs[0], * r2;

	jive_section_putbyte(target, 0xF3);
	jive_section_putbyte(target, 0x0F);
	
	if (icls->code == 0x11)
		r2 = inputs[1];
	else
		r2 = outputs[0];
	
	jive_section_putbyte(target, icls->code);
	
	jive_i386_r2i(r1, r2, &immediates[0], 3, flags, target);
	
}

static void
jive_i386_encode_sseload_abs(const jive_instruction_class * icls,
	jive_section * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	jive_section_putbyte(target, 0xF3);
	jive_section_putbyte(target, 0x0F);
	jive_section_putbyte(target, icls->code);
	jive_section_putbyte(target, 0x5 | outputs[0]->code << 3);

	jive_i386_encode_imm32(&immediates[0], 0, 4, target);
}

static void
jive_i386_encode_regreg_sse(const jive_instruction_class * icls,
	jive_section * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	JIVE_DEBUG_ASSERT(inputs[0] == outputs[0]);
	
	int r1 = inputs[0]->code;
	int r2 = inputs[1]->code;
	
	jive_section_putbyte(target, 0x0F);
	jive_section_putbyte(target, icls->code);
	jive_section_putbyte(target, 0xc0|r2|(r1<<3));
}

static void
jive_i386_encode_regreg_sse_prefixed(const jive_instruction_class * icls,
	jive_section * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	JIVE_DEBUG_ASSERT(inputs[0] == outputs[0]);
	
	jive_section_putbyte(target, 0xF3);
	jive_i386_encode_regreg_sse(icls, target, inputs, outputs, immediates, flags);
}

static void
jive_i386_asm_fp(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_asmgen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	jive_buffer_putstr(target, icls->mnemonic);
	jive_buffer_putstr(target, "\t");
	jive_buffer_putdisp(target, &immediates[0], inputs[0]);
}

static void
jive_i386_encode_fp(const jive_instruction_class * icls,
	jive_section * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	const jive_register_name * r1 = inputs[0];
	const jive_register_name * r2 = outputs[0];	

	jive_section_putbyte(target, 0xD9);
	
	jive_i386_r2i(r1, r2, &immediates[0], 1, flags, target);
}

static const jive_register_class * const fpreg_param[] = {
	&jive_i386_regcls[jive_i386_fp],
	&jive_i386_regcls[jive_i386_fp]
};

static const jive_register_class * const ssereg_param[] = {
	&jive_i386_regcls[jive_i386_sse],
	&jive_i386_regcls[jive_i386_sse]
};

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
		.inregs = 0, .outregs = 0, .flags = jive_instruction_jump, .ninputs = 0, .noutputs = 0, .nimmediates = 0,
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
		.write_asm = jive_i386_asm_load_disp,
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
		.encode = jive_i386_encode_div_reg,
		.write_asm = jive_i386_asm_div_reg,
		.inregs = (const jive_register_class *[]){&jive_i386_regcls[jive_i386_gpr_edx],
			&jive_i386_regcls[jive_i386_gpr_eax], &jive_i386_regcls[jive_i386_gpr]},
		.outregs = (const jive_register_class *[]){&jive_i386_regcls[jive_i386_gpr_edx],
			&jive_i386_regcls[jive_i386_gpr_eax], &jive_i386_regcls[jive_i386_flags]}, 
		.flags = jive_instruction_flags_none,
		.ninputs = 3, .noutputs = 3, .nimmediates = 0,
		.code = 0xf8
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
		.code = 0xf0
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
		.code = 0xf0 | (0x35 << 8)
	},
	[jive_i386_int_mul_immediate] = {
		.name = "int_mul_immediate",
		.mnemonic = "imull",
		.encode = jive_i386_encode_mul_regimm,
		.write_asm = jive_i386_asm_mul_regimm,
		.inregs = intreg_param, .outregs = intflags_param, .flags = jive_instruction_flags_none,
		.ninputs = 1, .noutputs = 2, .nimmediates = 1,
		.code = 0xc0 | (0x05 << 8)
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
		.ninputs = 1, .noutputs = 2, .nimmediates = 1,
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
		.ninputs = 1, .noutputs = 2, .nimmediates = 1,
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
		.ninputs = 1, .noutputs = 2, .nimmediates = 1,
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
		.encode = jive_i386_encode_cmp_regreg,
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
		.flags = jive_instruction_flags_none,
		.ninputs = 1, .noutputs = 1, .nimmediates = 1,
		.code = 0xf8 | (0x3d << 8)
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

	[jive_i386_fp_load_disp] = {
		.name = "fp_load_disp",
		.mnemonic = "flds",
		.encode = jive_i386_encode_fp,
		.write_asm = jive_i386_asm_fp,
		.inregs = intreg_param,
		.outregs = fpreg_param,
		.flags = jive_instruction_flags_none,
		.ninputs = 1, .noutputs = 1, .nimmediates = 1,
		.code = 0x0	
	},

	[jive_i386_sse_load32_disp] = {
		.name = "sse_load32_disp",
		.mnemonic = "movss",
		.encode = jive_i386_encode_loadstoresse_disp,
		.write_asm = jive_i386_asm_load_disp,
		.inregs = intreg_param,
		.outregs = ssereg_param,
		.flags = jive_instruction_flags_none,
		.ninputs = 1, .noutputs = 1, .nimmediates = 1,
		.code = 0x10
	},
	[jive_i386_sse_load_abs] = {
		.name = "sse_load_abs",
		.mnemonic = "movss",
		.encode = jive_i386_encode_sseload_abs,
		.write_asm = jive_i386_asm_load_abs,
		.inregs = NULL,
		.outregs = ssereg_param,
		.flags = jive_instruction_flags_none,
		.ninputs = 0, .noutputs = 1, .nimmediates = 1,
		.code = 0x10 
	},
	[jive_i386_sse_store32_disp] = {
		.name = "sse_store32_disp",
		.mnemonic = "movss",
		.encode = jive_i386_encode_loadstoresse_disp,
		.write_asm = jive_i386_asm_store,
		.inregs = (const jive_register_class *[]){
			&jive_i386_regcls[jive_i386_gpr], &jive_i386_regcls[jive_i386_sse]},
		.outregs = NULL,
		.flags = jive_instruction_flags_none,
		.ninputs = 2, .noutputs = 0, .nimmediates = 1,
		.code = 0x11
	},

	[jive_i386_sse_xor] = {
		.name = "xor",
		.mnemonic = "xorps",
		.encode = jive_i386_encode_regreg_sse,
		.write_asm = jive_i386_asm_regreg,
		.inregs = ssereg_param,
		.outregs = ssereg_param,
		.flags = jive_instruction_write_input | jive_instruction_commutative,
		.ninputs = 2, .noutputs = 1, .nimmediates = 0,
		.code = 0x57
	},

	[jive_i386_float_add] = {
		.name = "flt_add",
		.mnemonic = "addss",
		.encode = jive_i386_encode_regreg_sse_prefixed,
		.write_asm = jive_i386_asm_regreg,
		.inregs = ssereg_param, 
		.outregs = ssereg_param,
		.flags = jive_instruction_write_input | jive_instruction_commutative,
		.ninputs = 2, .noutputs = 1, .nimmediates = 0,
		.code = 0x58 
	},
	[jive_i386_float_sub] = {
		.name = "flt_sub",
		.mnemonic = "subss",
		.encode = jive_i386_encode_regreg_sse_prefixed,
		.write_asm = jive_i386_asm_regreg,
		.inregs = ssereg_param,
		.outregs = ssereg_param,
		.flags = jive_instruction_write_input,
		.ninputs = 2, .noutputs = 1, .nimmediates = 0,
		.code = 0x5C
	},
	[jive_i386_float_mul] = {
		.name = "flt_mul",
		.mnemonic = "mulss",
		.encode = jive_i386_encode_regreg_sse_prefixed,
		.write_asm = jive_i386_asm_regreg,
		.inregs = ssereg_param,
		.outregs = ssereg_param,
		.flags = jive_instruction_write_input | jive_instruction_commutative,
		.ninputs = 2, .noutputs = 1, .nimmediates = 0,
		.code = 0x59
	},
};

jive_xfer_description
jive_i386_create_xfer(struct jive_region * region, struct jive_output * origin,
	const struct jive_resource_class * in_class, const struct jive_resource_class * out_class);


static const jive_instructionset_class jive_i386_instructionset_class = {
	.create_xfer = jive_i386_create_xfer,
};

const struct jive_instructionset jive_i386_instructionset = {
	.class_ = &jive_i386_instructionset_class,
	.jump_instruction_class = &jive_i386_instructions[jive_i386_jump],
	.reg_classifier = &jive_i386_reg_classifier
};

JIVE_SERIALIZATION_INSTRSET_REGISTER(
	jive_i386_instructions,
	sizeof(jive_i386_instructions) / sizeof(jive_i386_instructions[0]),
	"i386_");