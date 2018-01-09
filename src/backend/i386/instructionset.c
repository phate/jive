/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/compilate.h>
#include <jive/arch/stackslot.h>
#include <jive/arch/subroutine/nodes.h>
#include <jive/backend/i386/classifier.h>
#include <jive/backend/i386/instructionset.h>
#include <jive/backend/i386/registerset.h>
#include <jive/backend/i386/relocation.h>
#include <jive/backend/i386/subroutine.h>
#include <jive/util/buffer.h>

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

static inline uint32_t
cpu_to_le32(uint32_t value)
{
	/* FIXME: endianness */
	return value;
}

static void
putimm(jive::buffer * target, const jive_asmgen_imm * imm)
{
	bool empty = true;
	if (imm->value) {
		target->push_back(jive::detail::strfmt(imm->value));
		empty = false;
	}
	if (imm->add_symbol) {
		if (!empty)
			target->push_back("+");
		target->push_back(imm->add_symbol);
		empty = false;
	}
	if (imm->sub_symbol) {
		target->push_back("-");
		target->push_back(imm->sub_symbol);
		empty = false;
	}
	if (empty) {
		target->push_back("0");
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
		*flags = *flags | jive_instruction_encoding_flags_option0;
	}
	
	return need_long_form;
}

static void
jive_i386_encode_imm8(
	const jive_codegen_imm * imm,
	jive_immediate_int offset,
	size_t coded_size_so_far,
	jive::section * target)
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
			target->putbyte(value);
			break;
		}
		case jive_codegen_imm_info_dynamic_unknown: {
			target->putbyte(0);
			break;
		}
		case jive_codegen_imm_info_static_unknown: {
			target->add_relocation(&value, 1, reltype, imm->symref, 0);
			break;
		}
	}
}

static void
jive_i386_encode_imm32(
	const jive_codegen_imm * imm,
	jive_immediate_int offset,
	size_t coded_size_so_far,
	jive::section * target)
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
			target->put(&value, sizeof(value));
			break;
		}
		case jive_codegen_imm_info_dynamic_unknown: {
			value = 0;
			target->put(&value, sizeof(value));
			break;
		}
		case jive_codegen_imm_info_static_unknown: {
			target->add_relocation(&value, sizeof(value), reltype, imm->symref, 0);
			break;
		}
	}
}

static void
putdisp(
	jive::buffer * target,
	const jive_asmgen_imm * disp,
	const jive::registers * reg)
{
	putimm(target, disp);
	target->push_back("(%");
	target->push_back(reg->name());
	target->push_back(")");
}

static void
jive_i386_encode_simple(
	const jive::instruction * icls,
	jive::section * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	target->putbyte(icls->code());
}

static void
jive_i386_asm_simple(
	const jive::instruction * icls,
	jive::buffer * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_asmgen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	target->push_back(icls->mnemonic());
}

static void
jive_i386_encode_int_load_imm(
	const jive::instruction * icls,
	jive::section * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	int reg = outputs[0]->code();
	target->putbyte(icls->code()|reg);
	jive_i386_encode_imm32(&immediates[0], 0, 1, target);
}

static void
jive_i386_asm_int_load_imm(
	const jive::instruction * icls,
	jive::buffer * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_asmgen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	target->push_back(icls->mnemonic());
	target->push_back("\t$");
	putimm(target, &immediates[0]);
	target->push_back(", %");
	target->push_back(outputs[0]->name());
}

static inline void
jive_i386_r2i(
	const jive::registers * r1,
	const jive::registers * r2,
	const jive_codegen_imm * imm,
	size_t coded_size_so_far,
	jive_instruction_encoding_flags * flags,
	jive::section * target)
{
	bool need_long_form = jive_i386_check_long_form(imm, flags, 0);
	
	int regcode = (r1->code()) | (r2->code()<<3);
	
	/* special treatment for load/store through ebp: always encode displacement parameter */
	bool code_displacement =
		(r1->code() == 5) ||
		need_long_form ||
		imm->value != 0 ||
		imm->info != jive_codegen_imm_info_static_known;
	
	if (code_displacement) {
		if (need_long_form)
			regcode |= 0x80;
		else
			regcode |= 0x40;
	}
	
	target->putbyte(regcode);
	coded_size_so_far ++;
	if (r1->code() == 4) {
		/* esp special treatment */
		target->putbyte(0x24);
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
jive_i386_encode_loadstore32_disp(
	const jive::instruction * icls,
	jive::section * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	const jive::registers * r1 = inputs[0], * r2;
	if (icls->code() == 0x89)
		r2 = inputs[1];
	else
		r2 = outputs[0];
	
	target->putbyte(icls->code());
	jive_i386_r2i(r1, r2, &immediates[0], 1, flags, target);
}

static void
jive_i386_asm_load_disp(
	const jive::instruction * icls,
	jive::buffer * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_asmgen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	target->push_back(icls->mnemonic());
	target->push_back("\t");
	putdisp(target, &immediates[0], inputs[0]);
	target->push_back(", %");
	target->push_back(outputs[0]->name());
}

static void
jive_i386_asm_load_abs(
	const jive::instruction * icls,
	jive::buffer * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_asmgen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	target->push_back(icls->mnemonic());
	target->push_back("\t");
	putimm(target, &immediates[0]);
	target->push_back(", %");
	target->push_back(outputs[0]->name());
}

static void
jive_i386_asm_store(
	const jive::instruction * icls,
	jive::buffer * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_asmgen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	target->push_back(icls->mnemonic());
	target->push_back("\t%");
	target->push_back(inputs[1]->name());
	target->push_back(", ");
	putdisp(target, &immediates[0], inputs[0]);
}

static void
jive_i386_encode_regreg(
	const jive::instruction * icls,
	jive::section * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	JIVE_DEBUG_ASSERT(inputs[0] == outputs[0]);
	
	int r1 = inputs[0]->code();
	int r2 = inputs[1]->code();
	
	target->putbyte(icls->code());
	target->putbyte(0xc0|r1|(r2<<3));
}

static void
jive_i386_asm_regreg(
	const jive::instruction * icls,
	jive::buffer * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_asmgen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	target->push_back(icls->mnemonic());
	target->push_back("\t%");
	target->push_back(inputs[1]->name());
	target->push_back(", %");
	target->push_back(inputs[0]->name());
}

static void
jive_i386_encode_cmp_regreg(
	const jive::instruction * icls,
	jive::section * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	int r1 = inputs[1]->code();
	int r2 = inputs[0]->code();
	
	target->putbyte(icls->code());
	target->putbyte(0xc0|r1|(r2<<3));
}

static void
jive_i386_encode_cmp_regreg_sse(
	const jive::instruction * icls,
	jive::section * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	target->putbyte(0x0F);
	jive_i386_encode_cmp_regreg(icls, target, inputs, outputs, immediates, flags);
}

static void
jive_i386_asm_imul(
	const jive::instruction * icls,
	jive::buffer * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_asmgen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	target->push_back(icls->mnemonic());
	target->push_back("\t%");
	target->push_back(inputs[1]->name());
	
	target->push_back(", %");
	target->push_back(inputs[0]->name());
}

static void
jive_i386_encode_imull(
	const jive::instruction * icls,
	jive::section * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	auto r1 = inputs[0]->code();
	auto r2 = inputs[1]->code();

	target->putbyte(icls->code());

	JIVE_DEBUG_ASSERT(r1 == outputs[1]->code());
	target->putbyte(0xe8|r2);
}

static void
jive_i386_encode_mul_regreg(
	const jive::instruction * icls,
	jive::section * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	auto r1 = inputs[0]->code();
	auto r2 = inputs[1]->code();
	
	JIVE_DEBUG_ASSERT(r1 == outputs[0]->code());
	
	target->putbyte(0x0f);
	target->putbyte(0xaf);
	target->putbyte(0xc0|r2|(r1<<3));
}

static void
jive_i386_asm_mul(
	const jive::instruction * icls,
	jive::buffer * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_asmgen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	target->push_back(icls->mnemonic());
	target->push_back("\t%");
	target->push_back(inputs[1]->name());
}

static void
jive_i386_encode_mull(
	const jive::instruction * icls,
	jive::section * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	auto r1 = inputs[0]->code();
	auto r2 = inputs[1]->code();

	JIVE_DEBUG_ASSERT(r1 == outputs[1]->code());

	target->putbyte(icls->code());
	target->putbyte(0xe0|r2);
}

static void
jive_i386_asm_div_reg(
	const jive::instruction * icls,
	jive::buffer * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_asmgen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	target->push_back(icls->mnemonic());
	target->push_back("\t%");
	target->push_back(inputs[2]->name());
}

static void
jive_i386_encode_div_reg(
	const jive::instruction * icls,
	jive::section * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	int r = inputs[2]->code();

	target->putbyte(0xf7);
	target->putbyte(icls->code() | r);
}

static void
jive_i386_encode_shift_regimm(
	const jive::instruction * icls,
	jive::section * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	auto r1 = inputs[0]->code();
	JIVE_DEBUG_ASSERT(r1 == outputs[0]->code());
	
	bool code_constant_one =
		immediates[0].info == jive_codegen_imm_info_static_known &&
		immediates[0].value == 1 &&
		immediates[0].symref.type == jive_symref_type_none;
	
	if (code_constant_one) {
		target->putbyte(0xd1);
		target->putbyte(icls->code() | r1);
	} else {
		target->putbyte(0xc1);
		target->putbyte(icls->code() | r1);
		jive_i386_encode_imm8(&immediates[0], 0, 2, target);
	}
}

static void
jive_i386_asm_shift_regreg(
	const jive::instruction * icls,
	jive::buffer * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_asmgen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	target->push_back(icls->mnemonic());
	target->push_back("\t%cl, %");
	target->push_back(inputs[0]->name());
}

static void
jive_i386_encode_shift_regreg(
	const jive::instruction * icls,
	jive::section * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	auto r1 = inputs[0]->code();
	JIVE_DEBUG_ASSERT(r1 == outputs[0]->code());
	
	target->putbyte(0xd3);
	target->putbyte(icls->code() | r1);
}

static void
jive_i386_encode_regimm_readonly(
	const jive::instruction * icls,
	jive::section * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	int r1 = inputs[0]->code();
	
	bool need_long_form = jive_i386_check_long_form(&immediates[0], flags, 0);
	size_t coded_size_so_far = 0;
	
	char prefix = need_long_form ? 0x81 : 0x83;
	
	if (r1 == 0 && need_long_form) {
		char opcode = icls->code() >> 8;
		target->putbyte(opcode);
		coded_size_so_far ++;
	} else {
		char opcode = icls->code() & 255;
		target->putbyte(prefix);
		target->putbyte(opcode | r1);
		coded_size_so_far += 2;
	}
	
	if (need_long_form)
		jive_i386_encode_imm32(&immediates[0], 0, coded_size_so_far, target);
	else
		jive_i386_encode_imm8(&immediates[0], 0, coded_size_so_far, target);
}

static void
jive_i386_encode_regimm(
	const jive::instruction * icls,
	jive::section * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	JIVE_DEBUG_ASSERT(inputs[0] == outputs[0]);
	jive_i386_encode_regimm_readonly(icls, target, inputs, outputs, immediates, flags);
}

static void
jive_i386_asm_regimm(
	const jive::instruction * icls,
	jive::buffer * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_asmgen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	target->push_back(icls->mnemonic());
	target->push_back("\t$");
	putimm(target, &immediates[0]);
	target->push_back(", %");
	target->push_back(inputs[0]->name());
}

static void
jive_i386_encode_mul_regimm(
	const jive::instruction * icls,
	jive::section * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	int r1 = inputs[0]->code();
	int r2 = outputs[0]->code();
	
	bool need_long_form = jive_i386_check_long_form(&immediates[0], flags, 0);
	
	char regcode = 0xc0 | (r2 << 3) | (r1 << 0);
	
	if (need_long_form) {
		target->putbyte(0x69);
		target->putbyte(regcode);
		jive_i386_encode_imm32(&immediates[0], 0, 2, target);
	} else {
		target->putbyte(0x6b);
		target->putbyte(regcode);
		jive_i386_encode_imm8(&immediates[0], 0, 2, target);
	}
}

static void
jive_i386_asm_mul_regimm(
	const jive::instruction * icls,
	jive::buffer * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_asmgen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	target->push_back(icls->mnemonic());
	target->push_back("\t$");
	putimm(target, &immediates[0]);
	target->push_back(", %");
	target->push_back(inputs[0]->name());
	target->push_back(", %");
	target->push_back(outputs[0]->name());
}

static void
jive_i386_encode_unaryreg(
	const jive::instruction * icls,
	jive::section * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	auto r1 = inputs[0]->code();
	
	JIVE_DEBUG_ASSERT(r1 == outputs[0]->code());
	
	target->putbyte(0xf7);
	target->putbyte(icls->code()|r1);
}

static void
jive_i386_asm_unaryreg(
	const jive::instruction * icls,
	jive::buffer * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_asmgen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	target->push_back(icls->mnemonic());
	target->push_back("\t%");
	target->push_back(inputs[0]->name());
}

static void
jive_i386_encode_regmove(
	const jive::instruction * icls,
	jive::section * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	int r1 = outputs[0]->code();
	int r2 = inputs[0]->code();
	
	target->putbyte(icls->code());
	target->putbyte(0xc0|r1|(r2<<3));
}

static void
jive_i386_encode_regmove_sse(
	const jive::instruction * icls,
	jive::section * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	target->putbyte(0xF3);
	target->putbyte(0x0F);
	jive_i386_encode_regmove(icls, target, outputs, inputs, immediates, flags);
}

static void
jive_i386_asm_regmove(
	const jive::instruction * icls,
	jive::buffer * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_asmgen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	target->push_back(icls->mnemonic());
	target->push_back("\t%");
	target->push_back(inputs[0]->name());
	target->push_back(", %");
	target->push_back(outputs[0]->name());
}

static void
jive_i386_encode_call(
	const jive::instruction * icls,
	jive::section * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	target->putbyte(icls->code());
	jive_i386_encode_imm32(&immediates[0], 0, 1, target);
}

static void
jive_i386_asm_call(
	const jive::instruction * icls,
	jive::buffer * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_asmgen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	target->push_back(icls->mnemonic());
	target->push_back("\t");
	putimm(target, &immediates[0]);
}

static void
jive_i386_encode_call_reg(
	const jive::instruction * icls,
	jive::section * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	int r = inputs[0]->code();
	
	target->putbyte(0xff);
	target->putbyte(0xd0 | r);
}

static void
jive_i386_asm_call_reg(
	const jive::instruction * icls,
	jive::buffer * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_asmgen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	target->push_back(icls->mnemonic());
	target->push_back("\t*%");
	target->push_back(inputs[0]->name());
}

static void
jive_i386_asm_jump(
	const jive::instruction * icls,
	jive::buffer * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_asmgen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	target->push_back(icls->mnemonic());
	target->push_back("\t");
	putimm(target, &immediates[0]);
}

static void
jive_i386_encode_jump(
	const jive::instruction * icls,
	jive::section * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	bool need_long_form = jive_i386_check_long_form(&immediates[0], flags, -2);
	
	if (!need_long_form) {
		target->putbyte(0xeb);
		jive_i386_encode_imm8(&immediates[0], -2, 1, target);
	} else {
		target->putbyte(0xe9);
		jive_i386_encode_imm32(&immediates[0], -5, 1, target);
	}
}

static void
jive_i386_encode_jump_conditional(
	const jive::instruction * icls,
	jive::section * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	bool need_long_form = jive_i386_check_long_form(&immediates[0], flags, -2);
	
	if (!need_long_form) {
		target->putbyte(0x70 | icls->code());
		jive_i386_encode_imm8(&immediates[0], -2, 1, target);
	} else {
		target->putbyte(0x0f);
		target->putbyte(0x80 | icls->code());
		jive_i386_encode_imm32(&immediates[0], -6, 2, target);
	}
}

static void
jive_i386_encode_loadstoresse_disp(
	const jive::instruction * icls,
	jive::section * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	const jive::registers * r1 = inputs[0], * r2;

	target->putbyte(0xF3);
	target->putbyte(0x0F);
	
	if (icls->code() == 0x11)
		r2 = inputs[1];
	else
		r2 = outputs[0];
	
	target->putbyte(icls->code());
	jive_i386_r2i(r1, r2, &immediates[0], 3, flags, target);
	
}

static void
jive_i386_encode_sseload_abs(
	const jive::instruction * icls,
	jive::section * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	target->putbyte(0xF3);
	target->putbyte(0x0F);
	target->putbyte(icls->code());
	target->putbyte(0x5 | outputs[0]->code() << 3);

	jive_i386_encode_imm32(&immediates[0], 0, 4, target);
}

static void
jive_i386_encode_regreg_sse(
	const jive::instruction * icls,
	jive::section * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	JIVE_DEBUG_ASSERT(inputs[0] == outputs[0]);
	
	int r1 = inputs[0]->code();
	int r2 = inputs[1]->code();
	
	target->putbyte(0x0F);
	target->putbyte(icls->code());
	target->putbyte(0xc0|r2|(r1<<3));
}

static void
jive_i386_encode_regreg_sse_prefixed(
	const jive::instruction * icls,
	jive::section * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	JIVE_DEBUG_ASSERT(inputs[0] == outputs[0]);
	
	target->putbyte(0xF3);
	jive_i386_encode_regreg_sse(icls, target, inputs, outputs, immediates, flags);
}

static void
jive_i386_asm_fp(
	const jive::instruction * icls,
	jive::buffer * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_asmgen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	target->push_back(icls->mnemonic());
	target->push_back("\t");
	putdisp(target, &immediates[0], inputs[0]);
}

static void
jive_i386_encode_fp(
	const jive::instruction * icls,
	jive::section * target,
	const jive::registers * inputs[],
	const jive::registers * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
	const jive::registers * r1 = inputs[0];
	const jive::registers * r2 = outputs[0];

	target->putbyte(0xD9);
	
	jive_i386_r2i(r1, r2, &immediates[0], 1, flags, target);
}

static void
get_slot_memory_reference(const jive::resource_class * rescls,
	jive::immediate * displacement, jive::output ** base,
	jive::output * sp, jive::output * fp)
{
	if (rescls->is_resource(&callslot_resource)) {
		*displacement = jive::immediate(0, jive::spoffset_label::get());
		*base = sp;
	} else {
		*displacement = jive::immediate(0, jive::fpoffset_label::get());
		*base = fp;
	}
}

namespace jive {
namespace i386 {

/* FIXME: encode and write_asm take nullptr as first argument */
#define DEFINE_I386_INSTRUCTION(NAME, CODE, MNEMONIC, \
	INPUTS, OUTPUTS, NIMMEDIATES, FLAGS, INVERSE_JUMP, \
	ENCODE, WRITE_ASM) \
const instr_##NAME instr_##NAME::instance_; \
 \
instr_##NAME::instr_##NAME() \
	: instruction(#NAME, CODE, MNEMONIC, \
		INPUTS, OUTPUTS, NIMMEDIATES, \
		FLAGS, INVERSE_JUMP) \
	{} \
\
void \
instr_##NAME::encode( \
	jive::section * target, \
	const jive::registers * inputs[], \
	const jive::registers * outputs[], \
	const jive_codegen_imm immediates[], \
	jive_instruction_encoding_flags * flags) const \
{ \
	ENCODE(nullptr, target, inputs, outputs, immediates, flags); \
} \
 \
void \
instr_##NAME::write_asm( \
	jive::buffer * target, \
	const jive::registers * inputs[], \
	const jive::registers * outputs[], \
	const jive_asmgen_imm immediates[], \
	jive_instruction_encoding_flags * flags) const \
{ \
	WRITE_ASM(nullptr, target, inputs, outputs, immediates, flags); \
} \
 \
std::unique_ptr<jive::instruction> \
instr_##NAME::copy() const \
{ \
	return std::make_unique<jive::i386::instr_##NAME>(); \
} \

#define COMMA ,

DEFINE_I386_INSTRUCTION(
	ret, 0xC3, "ret",
	{}, {}, 0,
	jive_instruction_jump, nullptr,
	jive_i386_encode_simple, jive_i386_asm_simple)

/* integer load, store, and move instructions */
DEFINE_I386_INSTRUCTION(
	int_load_imm, 0x8B, "movl",
	{}, {&jive_i386_regcls_gpr}, 1,
	jive_instruction_flags_none, nullptr,
	jive_i386_encode_int_load_imm, jive_i386_asm_int_load_imm)
DEFINE_I386_INSTRUCTION(
	int_load32_disp, 0x8B, "movl",
	{&jive_i386_regcls_gpr}, {&jive_i386_regcls_gpr}, 1,
	jive_instruction_flags_none, nullptr,
	jive_i386_encode_loadstore32_disp, jive_i386_asm_load_disp)
DEFINE_I386_INSTRUCTION(
	int_store32_disp, 0x89, "movl",
	{&jive_i386_regcls_gpr COMMA &jive_i386_regcls_gpr}, {}, 1,
	jive_instruction_flags_none, nullptr,
	jive_i386_encode_loadstore32_disp, jive_i386_asm_store)
DEFINE_I386_INSTRUCTION(
	int_transfer, 0x89, "movl",
	{&jive_i386_regcls_gpr}, {&jive_i386_regcls_gpr}, 0,
	jive_instruction_flags_none, nullptr,
	jive_i386_encode_regmove, jive_i386_asm_regmove)

/* integer arithmetic register register instructions */
DEFINE_I386_INSTRUCTION(
	int_add, 0x01, "addl",
	{&jive_i386_regcls_gpr COMMA &jive_i386_regcls_gpr},
	{&jive_i386_regcls_gpr COMMA &jive_i386_regcls_flags}, 0,
	jive_instruction_write_input | jive_instruction_commutative, nullptr,
	jive_i386_encode_regreg, jive_i386_asm_regreg)
DEFINE_I386_INSTRUCTION(
	int_sub, 0x29, "subl",
	{&jive_i386_regcls_gpr COMMA &jive_i386_regcls_gpr},
	{&jive_i386_regcls_gpr COMMA &jive_i386_regcls_flags}, 0,
	jive_instruction_write_input, nullptr,
	jive_i386_encode_regreg, jive_i386_asm_regreg)
DEFINE_I386_INSTRUCTION(
	int_and, 0x21, "andl",
	{&jive_i386_regcls_gpr COMMA &jive_i386_regcls_gpr},
	{&jive_i386_regcls_gpr COMMA &jive_i386_regcls_flags}, 0,
	jive_instruction_write_input | jive_instruction_commutative, nullptr,
	jive_i386_encode_regreg, jive_i386_asm_regreg)
DEFINE_I386_INSTRUCTION(
	int_or, 0x09, "orl",
	{&jive_i386_regcls_gpr COMMA &jive_i386_regcls_gpr},
	{&jive_i386_regcls_gpr COMMA &jive_i386_regcls_flags}, 0,
	jive_instruction_write_input | jive_instruction_commutative, nullptr,
	jive_i386_encode_regreg, jive_i386_asm_regreg)
DEFINE_I386_INSTRUCTION(
	int_xor, 0x31, "xorl",
	{&jive_i386_regcls_gpr COMMA &jive_i386_regcls_gpr},
	{&jive_i386_regcls_gpr COMMA &jive_i386_regcls_flags}, 0,
	jive_instruction_write_input | jive_instruction_commutative, nullptr,
	jive_i386_encode_regreg, jive_i386_asm_regreg)
DEFINE_I386_INSTRUCTION(
	int_mul, 0xC0AF0F, "imull",
	{&jive_i386_regcls_gpr COMMA &jive_i386_regcls_gpr},
	{&jive_i386_regcls_gpr COMMA &jive_i386_regcls_flags}, 0,
	jive_instruction_write_input | jive_instruction_commutative, nullptr,
	jive_i386_encode_mul_regreg, jive_i386_asm_regreg)
DEFINE_I386_INSTRUCTION(
	int_mul_expand_signed, 0xF7, "imull",
	{&jive_i386_regcls_gpr_eax COMMA &jive_i386_regcls_gpr},
	{&jive_i386_regcls_gpr_edx COMMA &jive_i386_regcls_gpr_eax COMMA &jive_i386_regcls_flags}, 0,
	jive_instruction_commutative, nullptr,
	jive_i386_encode_imull, jive_i386_asm_imul)
DEFINE_I386_INSTRUCTION(
	int_mul_expand_unsigned, 0xF7, "mull",
	{&jive_i386_regcls_gpr_eax COMMA &jive_i386_regcls_gpr},
	{&jive_i386_regcls_gpr_edx COMMA &jive_i386_regcls_gpr_eax COMMA &jive_i386_regcls_flags}, 0,
	jive_instruction_commutative, nullptr,
	jive_i386_encode_mull, jive_i386_asm_mul)
DEFINE_I386_INSTRUCTION(
	int_sdiv, 0xF8, "idivl",
	{&jive_i386_regcls_gpr_edx COMMA &jive_i386_regcls_gpr_eax COMMA &jive_i386_regcls_gpr},
	{&jive_i386_regcls_gpr_edx COMMA &jive_i386_regcls_gpr_eax COMMA &jive_i386_regcls_flags}, 0,
	jive_instruction_flags_none, nullptr,
	jive_i386_encode_div_reg, jive_i386_asm_div_reg)
DEFINE_I386_INSTRUCTION(
	int_udiv, 0xF0, "divl",
	{&jive_i386_regcls_gpr_edx COMMA &jive_i386_regcls_gpr_eax COMMA &jive_i386_regcls_gpr},
	{&jive_i386_regcls_gpr_edx COMMA &jive_i386_regcls_gpr_eax COMMA &jive_i386_regcls_flags}, 0,
	jive_instruction_flags_none, nullptr,
	jive_i386_encode_div_reg, jive_i386_asm_div_reg)
DEFINE_I386_INSTRUCTION(
	int_neg, 0xD8, "negl",
	{&jive_i386_regcls_gpr}, {&jive_i386_regcls_gpr COMMA &jive_i386_regcls_flags}, 0,
	jive_instruction_write_input, nullptr,
	jive_i386_encode_unaryreg, jive_i386_asm_unaryreg)
DEFINE_I386_INSTRUCTION(
	int_not, 0xD0, "notl",
	{&jive_i386_regcls_gpr}, {&jive_i386_regcls_gpr COMMA &jive_i386_regcls_flags}, 0,
	jive_instruction_write_input, nullptr,
	jive_i386_encode_unaryreg, jive_i386_asm_unaryreg)
DEFINE_I386_INSTRUCTION(
	int_shr, 0xE8, "shrl",
	{&jive_i386_regcls_gpr COMMA &jive_i386_regcls_gpr_ecx},
	{&jive_i386_regcls_gpr COMMA &jive_i386_regcls_flags}, 0,
	jive_instruction_write_input, nullptr,
	jive_i386_encode_shift_regreg, jive_i386_asm_shift_regreg)
DEFINE_I386_INSTRUCTION(
	int_shl, 0xE0, "shll",
	{&jive_i386_regcls_gpr COMMA &jive_i386_regcls_gpr_ecx},
	{&jive_i386_regcls_gpr COMMA &jive_i386_regcls_flags}, 0,
	jive_instruction_write_input, nullptr,
	jive_i386_encode_shift_regreg, jive_i386_asm_shift_regreg)
DEFINE_I386_INSTRUCTION(
	int_ashr, 0xF8, "sarl",
	{&jive_i386_regcls_gpr COMMA &jive_i386_regcls_gpr_ecx},
	{&jive_i386_regcls_gpr COMMA &jive_i386_regcls_flags}, 0,
	jive_instruction_write_input, nullptr,
	jive_i386_encode_shift_regreg, jive_i386_asm_shift_regreg)

/* integer arithmetic register immediate instructions */
/*
	for the immediate instructions, code consists of normal_code | (eax_cod << 8), see instruction
	coding function for explanation
*/
DEFINE_I386_INSTRUCTION(
	int_add_immediate, 0xC0 | (0x05 << 8), "addl",
	{&jive_i386_regcls_gpr}, {&jive_i386_regcls_gpr COMMA &jive_i386_regcls_flags}, 1,
	jive_instruction_write_input, nullptr,
	jive_i386_encode_regimm, jive_i386_asm_regimm)
DEFINE_I386_INSTRUCTION(
	int_sub_immediate, 0xE8 | (0x2D << 8), "subl",
	{&jive_i386_regcls_gpr}, {&jive_i386_regcls_gpr COMMA &jive_i386_regcls_flags}, 1,
	jive_instruction_write_input, nullptr,
	jive_i386_encode_regimm, jive_i386_asm_regimm)
DEFINE_I386_INSTRUCTION(
	int_and_immediate, 0xE0 | (0x25 << 8), "andl",
	{&jive_i386_regcls_gpr}, {&jive_i386_regcls_gpr COMMA &jive_i386_regcls_flags}, 1,
	jive_instruction_write_input, nullptr,
	jive_i386_encode_regimm, jive_i386_asm_regimm)
DEFINE_I386_INSTRUCTION(
	int_or_immediate, 0xC8 | (0x0D << 8), "orl",
	{&jive_i386_regcls_gpr}, {&jive_i386_regcls_gpr COMMA &jive_i386_regcls_flags}, 1,
	jive_instruction_write_input, nullptr,
	jive_i386_encode_regimm, jive_i386_asm_regimm)
DEFINE_I386_INSTRUCTION(
	int_xor_immediate, 0xF0 | (0x35 << 8), "xorl",
	{&jive_i386_regcls_gpr}, {&jive_i386_regcls_gpr COMMA &jive_i386_regcls_flags}, 1,
	jive_instruction_write_input, nullptr,
	jive_i386_encode_regimm, jive_i386_asm_regimm)
DEFINE_I386_INSTRUCTION(
	/* FIXME: code is the same as for int_add_immediate, flags is none */
	int_mul_immediate, 0xC0 | (0x05 << 8), "imull",
	{&jive_i386_regcls_gpr}, {&jive_i386_regcls_gpr COMMA &jive_i386_regcls_flags}, 1,
	jive_instruction_flags_none, nullptr,
	jive_i386_encode_mul_regimm, jive_i386_asm_mul_regimm)
DEFINE_I386_INSTRUCTION(
	int_shr_immediate, 0xE8, "shrl",
	{&jive_i386_regcls_gpr}, {&jive_i386_regcls_gpr COMMA &jive_i386_regcls_flags}, 1,
	jive_instruction_write_input, nullptr,
	jive_i386_encode_shift_regimm, jive_i386_asm_regimm)
DEFINE_I386_INSTRUCTION(
	int_shl_immediate, 0xE0, "shll",
	{&jive_i386_regcls_gpr}, {&jive_i386_regcls_gpr COMMA &jive_i386_regcls_flags}, 1,
	jive_instruction_write_input, nullptr,
	jive_i386_encode_shift_regimm, jive_i386_asm_regimm)
DEFINE_I386_INSTRUCTION(
	int_ashr_immediate, 0xF8, "sarl",
	{&jive_i386_regcls_gpr}, {&jive_i386_regcls_gpr COMMA &jive_i386_regcls_flags}, 1,
	jive_instruction_write_input, nullptr,
	jive_i386_encode_shift_regimm, jive_i386_asm_regimm)

/* call instructions */
DEFINE_I386_INSTRUCTION(
	call, 0xE8, "call",
	{}, {}, 1,
	jive_instruction_flags_none, nullptr,
	jive_i386_encode_call, jive_i386_asm_call)
DEFINE_I386_INSTRUCTION(
	call_reg, 0xFF, "call_reg",
	{&jive_i386_regcls_gpr}, {}, 0,
	jive_instruction_flags_none, nullptr,
	jive_i386_encode_call_reg, jive_i386_asm_call_reg)

/* integer compare instructions */
DEFINE_I386_INSTRUCTION(
	int_cmp, 0x3b, "cmpl",
	{&jive_i386_regcls_gpr COMMA &jive_i386_regcls_gpr}, {&jive_i386_regcls_flags}, 0,
	jive_instruction_flags_none, nullptr,
	jive_i386_encode_cmp_regreg, jive_i386_asm_regreg)
DEFINE_I386_INSTRUCTION(
	int_cmp_immediate, 0xF8 | (0x3D << 8), "cmpl",
	{&jive_i386_regcls_gpr}, {&jive_i386_regcls_flags}, 1,
	jive_instruction_flags_none, nullptr,
	jive_i386_encode_regimm_readonly, jive_i386_asm_regimm)

/* jump instructions */
DEFINE_I386_INSTRUCTION(
	int_jump_sless, 0xC, "jl",
	{&jive_i386_regcls_flags}, {}, 1,
	jive_instruction_jump | jive_instruction_jump_relative
	| jive_instruction_jump_conditional_invertible, &instr_int_jump_sgreatereq::instance(),
	jive_i386_encode_jump_conditional, jive_i386_asm_jump)
DEFINE_I386_INSTRUCTION(
	int_jump_uless, 0x2, "jb",
	{&jive_i386_regcls_flags}, {}, 1,
	jive_instruction_jump | jive_instruction_jump_relative
	| jive_instruction_jump_conditional_invertible, &instr_int_jump_ugreatereq::instance(),
	jive_i386_encode_jump_conditional, jive_i386_asm_jump)
DEFINE_I386_INSTRUCTION(
	int_jump_slesseq, 0xE, "jle",
	{&jive_i386_regcls_flags}, {}, 1,
	jive_instruction_jump | jive_instruction_jump_relative
	| jive_instruction_jump_conditional_invertible, &instr_int_jump_sgreater::instance(),
	jive_i386_encode_jump_conditional, jive_i386_asm_jump)
DEFINE_I386_INSTRUCTION(
	int_jump_ulesseq, 0x6, "jbe",
	{&jive_i386_regcls_flags}, {}, 1,
	jive_instruction_jump | jive_instruction_jump_relative
	| jive_instruction_jump_conditional_invertible, &instr_int_jump_ugreater::instance(),
	jive_i386_encode_jump_conditional, jive_i386_asm_jump)
DEFINE_I386_INSTRUCTION(
	int_jump_equal, 0x4, "je",
	{&jive_i386_regcls_flags}, {}, 1,
	jive_instruction_jump | jive_instruction_jump_relative
	| jive_instruction_jump_conditional_invertible, &instr_int_jump_notequal::instance(),
	jive_i386_encode_jump_conditional, jive_i386_asm_jump)
DEFINE_I386_INSTRUCTION(
	int_jump_notequal, 0x5, "jne",
	{&jive_i386_regcls_flags}, {}, 1,
	jive_instruction_jump | jive_instruction_jump_relative
	| jive_instruction_jump_conditional_invertible, &instr_int_jump_equal::instance(),
	jive_i386_encode_jump_conditional, jive_i386_asm_jump)
DEFINE_I386_INSTRUCTION(
	int_jump_sgreater, 0xF, "jg",
	{&jive_i386_regcls_flags}, {}, 1,
	jive_instruction_jump | jive_instruction_jump_relative
	| jive_instruction_jump_conditional_invertible, &instr_int_jump_slesseq::instance(),
	jive_i386_encode_jump_conditional, jive_i386_asm_jump)
DEFINE_I386_INSTRUCTION(
	int_jump_ugreater, 0x7, "ja",
	{&jive_i386_regcls_flags}, {}, 1,
	jive_instruction_jump | jive_instruction_jump_relative
	| jive_instruction_jump_conditional_invertible, &instr_int_jump_ulesseq::instance(),
	jive_i386_encode_jump_conditional, jive_i386_asm_jump)
DEFINE_I386_INSTRUCTION(
	int_jump_sgreatereq, 0xD, "jge",
	{&jive_i386_regcls_flags}, {}, 1,
	jive_instruction_jump | jive_instruction_jump_relative
	| jive_instruction_jump_conditional_invertible, &instr_int_jump_sless::instance(),
	jive_i386_encode_jump_conditional, jive_i386_asm_jump)
DEFINE_I386_INSTRUCTION(
	int_jump_ugreatereq, 0x3, "jae",
	{&jive_i386_regcls_flags}, {}, 1,
	jive_instruction_jump | jive_instruction_jump_relative
	| jive_instruction_jump_conditional_invertible, &instr_int_jump_uless::instance(),
	jive_i386_encode_jump_conditional, jive_i386_asm_jump)
DEFINE_I386_INSTRUCTION(
	jump, 0xEB, "jmp",
	{}, {}, 1,
	jive_instruction_jump_relative, nullptr,
	jive_i386_encode_jump, jive_i386_asm_jump)

/* floating-point instructions */
DEFINE_I386_INSTRUCTION(
	fp_load_disp, 0x0, "flds",
	{&jive_i386_regcls_gpr}, {&jive_i386_regcls_fp}, 1,
	jive_instruction_flags_none, nullptr,
	jive_i386_encode_fp, jive_i386_asm_fp)
DEFINE_I386_INSTRUCTION(
	sse_load32_disp, 0x10, "movss",
	{&jive_i386_regcls_gpr}, {&jive_i386_regcls_sse}, 1,
	jive_instruction_flags_none, nullptr,
	jive_i386_encode_loadstoresse_disp, jive_i386_asm_load_disp)
DEFINE_I386_INSTRUCTION(
	sse_load_abs, 0x10, "movss",
	{}, {&jive_i386_regcls_sse}, 1,
	jive_instruction_flags_none, nullptr,
	jive_i386_encode_sseload_abs, jive_i386_asm_load_abs)
DEFINE_I386_INSTRUCTION(
	sse_store32_disp, 0x11, "movss",
	{&jive_i386_regcls_gpr COMMA &jive_i386_regcls_sse}, {}, 1,
	jive_instruction_flags_none, nullptr,
	jive_i386_encode_loadstoresse_disp, jive_i386_asm_store)
DEFINE_I386_INSTRUCTION(
	sse_xor, 0x57, "xorps",
	{&jive_i386_regcls_sse COMMA &jive_i386_regcls_sse}, {&jive_i386_regcls_sse}, 0,
	jive_instruction_write_input | jive_instruction_commutative, nullptr,
	jive_i386_encode_regreg_sse, jive_i386_asm_regreg)
DEFINE_I386_INSTRUCTION(
	float_add, 0x58, "addss",
	{&jive_i386_regcls_sse COMMA &jive_i386_regcls_sse}, {&jive_i386_regcls_sse}, 0,
	jive_instruction_write_input | jive_instruction_commutative, nullptr,
	jive_i386_encode_regreg_sse_prefixed, jive_i386_asm_regreg)
DEFINE_I386_INSTRUCTION(
	float_sub, 0x5C, "subss",
	{&jive_i386_regcls_sse COMMA &jive_i386_regcls_sse}, {&jive_i386_regcls_sse}, 0,
	jive_instruction_write_input, nullptr,
	jive_i386_encode_regreg_sse_prefixed, jive_i386_asm_regreg)
DEFINE_I386_INSTRUCTION(
	float_mul, 0x59, "mulss",
	{&jive_i386_regcls_sse COMMA &jive_i386_regcls_sse}, {&jive_i386_regcls_sse}, 0,
	jive_instruction_write_input | jive_instruction_commutative, nullptr,
	jive_i386_encode_regreg_sse_prefixed, jive_i386_asm_regreg)
DEFINE_I386_INSTRUCTION(
	float_div, 0x5E, "divss",
	{&jive_i386_regcls_sse COMMA &jive_i386_regcls_sse}, {&jive_i386_regcls_sse}, 0,
	jive_instruction_write_input, nullptr,
	jive_i386_encode_regreg_sse_prefixed, jive_i386_asm_regreg)
DEFINE_I386_INSTRUCTION(
	float_cmp, 0x2E, "ucomiss",
	{&jive_i386_regcls_sse COMMA &jive_i386_regcls_sse}, {&jive_i386_regcls_flags}, 0,
	jive_instruction_flags_none, nullptr,
	jive_i386_encode_cmp_regreg_sse, jive_i386_asm_regreg)
DEFINE_I386_INSTRUCTION(
	float_transfer, 0x10, "movss",
	{&jive_i386_regcls_sse}, {&jive_i386_regcls_sse}, 0,
	jive_instruction_flags_none, nullptr,
	jive_i386_encode_regmove_sse, jive_i386_asm_regmove)

/* instructionset */

instructionset::~instructionset()
{}

const jive::instruction *
instructionset::jump_instruction() const noexcept
{
	return &jive::i386::instr_jump::instance();
}

const jive::register_classifier *
instructionset::classifier() const noexcept
{
	return register_classifier::get();
}

jive::xfer_description
instructionset::create_xfer(
	jive::region * region,
	jive::output * origin,
	const jive::resource_class * in_class,
	const jive::resource_class * out_class)
{
	auto sub = jive_region_get_subroutine_node(region);

	auto sp = jive_subroutine_node_get_sp(sub);
	auto fp = jive_subroutine_node_get_fp(sub);

	if (!in_class->is_resource(&jive::register_resource)) {
		jive::output * base;
		jive::immediate displacement;
		get_slot_memory_reference(in_class, &displacement, &base, sp, fp);
		auto imm = jive_immediate_create(region, &displacement);
		auto node = jive::create_instruction(region, &jive::i386::instr_int_load32_disp::instance(),
			{base, imm, origin}, {in_class}, {});
		return jive::xfer_description(node->input(2), node, node->output(0));
	}

	if (!out_class->is_resource(&jive::register_resource)) {
		jive::output * base;
		jive::immediate displacement;
		get_slot_memory_reference(out_class, &displacement, &base, sp, fp);
		auto imm = jive_immediate_create(region, &displacement);
		auto node = jive::create_instruction(region, &jive::i386::instr_int_store32_disp::instance(),
			{base, origin, imm}, {}, {out_class});
		return jive::xfer_description(node->input(1), node, node->output(0));
	}

	auto node = jive::create_instruction(region, &jive::i386::instr_int_transfer::instance(),
		{origin});
	return jive::xfer_description(node->input(0), node, node->output(0));
}

}}
