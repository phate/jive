/*
 * Copyright 2010 2011 2012 2013 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_BACKEND_I386_INSTRUCTIONSET_H
#define JIVE_BACKEND_I386_INSTRUCTIONSET_H

#include <jive/arch/instruction.h>

namespace jive {
namespace i386 {

#define DECLARE_I386_INSTRUCTION(NAME) \
class instr_##NAME : public jive::instruction_class { \
public: \
	instr_##NAME(); \
\
	virtual void \
	encode(	\
		struct jive_section * target, \
		const jive_register_name * inputs[], \
		const jive_register_name * outputs[], \
		const jive_codegen_imm immediates[], \
		jive_instruction_encoding_flags * flags) const override; \
\
	virtual void \
	write_asm( \
		struct jive_buffer * target, \
		const jive_register_name * inputs[], \
		const jive_register_name * outputs[], \
		const jive_asmgen_imm immediates[], \
		jive_instruction_encoding_flags * flags) const override; \
\
	virtual std::unique_ptr<jive::instruction_class> \
	copy() const override; \
\
	static const instr_##NAME & \
	instance() \
	{ \
		return instance_; \
	} \
\
private: \
	static const instr_##NAME instance_; \
} \

DECLARE_I386_INSTRUCTION(ret);

DECLARE_I386_INSTRUCTION(int_load_imm);
DECLARE_I386_INSTRUCTION(int_load32_disp);
DECLARE_I386_INSTRUCTION(int_store32_disp);

DECLARE_I386_INSTRUCTION(int_add);
DECLARE_I386_INSTRUCTION(int_sub);
DECLARE_I386_INSTRUCTION(int_and);
DECLARE_I386_INSTRUCTION(int_or);
DECLARE_I386_INSTRUCTION(int_xor);
DECLARE_I386_INSTRUCTION(int_mul);
DECLARE_I386_INSTRUCTION(int_sdiv);
DECLARE_I386_INSTRUCTION(int_udiv);
DECLARE_I386_INSTRUCTION(int_shl);
DECLARE_I386_INSTRUCTION(int_shr);
DECLARE_I386_INSTRUCTION(int_ashr);
DECLARE_I386_INSTRUCTION(int_mul_expand_signed);
DECLARE_I386_INSTRUCTION(int_mul_expand_unsigned);

DECLARE_I386_INSTRUCTION(int_add_immediate);
DECLARE_I386_INSTRUCTION(int_sub_immediate);
DECLARE_I386_INSTRUCTION(int_and_immediate);
DECLARE_I386_INSTRUCTION(int_or_immediate);
DECLARE_I386_INSTRUCTION(int_xor_immediate);
DECLARE_I386_INSTRUCTION(int_mul_immediate);
DECLARE_I386_INSTRUCTION(int_shl_immediate);
DECLARE_I386_INSTRUCTION(int_shr_immediate);
DECLARE_I386_INSTRUCTION(int_ashr_immediate);

DECLARE_I386_INSTRUCTION(int_neg);
DECLARE_I386_INSTRUCTION(int_not);

DECLARE_I386_INSTRUCTION(int_transfer);

DECLARE_I386_INSTRUCTION(call);
DECLARE_I386_INSTRUCTION(call_reg);

DECLARE_I386_INSTRUCTION(int_cmp);
DECLARE_I386_INSTRUCTION(int_cmp_immediate);

DECLARE_I386_INSTRUCTION(int_jump_sless);
DECLARE_I386_INSTRUCTION(int_jump_uless);
DECLARE_I386_INSTRUCTION(int_jump_slesseq);
DECLARE_I386_INSTRUCTION(int_jump_ulesseq);
DECLARE_I386_INSTRUCTION(int_jump_equal);
DECLARE_I386_INSTRUCTION(int_jump_notequal);
DECLARE_I386_INSTRUCTION(int_jump_sgreater);
DECLARE_I386_INSTRUCTION(int_jump_ugreater);
DECLARE_I386_INSTRUCTION(int_jump_sgreatereq);
DECLARE_I386_INSTRUCTION(int_jump_ugreatereq);
DECLARE_I386_INSTRUCTION(jump);

DECLARE_I386_INSTRUCTION(fp_load_disp);
DECLARE_I386_INSTRUCTION(sse_load32_disp);
DECLARE_I386_INSTRUCTION(sse_load_abs);
DECLARE_I386_INSTRUCTION(sse_store32_disp);
DECLARE_I386_INSTRUCTION(sse_xor);
DECLARE_I386_INSTRUCTION(float_add);
DECLARE_I386_INSTRUCTION(float_sub);
DECLARE_I386_INSTRUCTION(float_mul);
DECLARE_I386_INSTRUCTION(float_div);
DECLARE_I386_INSTRUCTION(float_cmp);
DECLARE_I386_INSTRUCTION(float_transfer);

}}

struct jive_instructionset;
extern const struct jive_instructionset jive_i386_instructionset;

#endif
