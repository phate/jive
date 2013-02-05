/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_BACKEND_I386_INSTRUCTIONSET_H
#define JIVE_BACKEND_I386_INSTRUCTIONSET_H

#include <jive/arch/instruction.h>

struct jive_instructionset;

typedef enum {
	jive_i386_ret = 0,
	
	jive_i386_int_load_imm = 1,
	jive_i386_int_load32_disp = 2,
	jive_i386_int_store32_disp = 3,
	
	jive_i386_int_add = 16,
	jive_i386_int_sub = 17,
	jive_i386_int_and = 18,
	jive_i386_int_or = 19,
	jive_i386_int_xor = 20,
	jive_i386_int_mul = 21,
	jive_i386_int_sdiv = 22,
	jive_i386_int_udiv = 23,
	jive_i386_int_shl = 24,	
	jive_i386_int_shr = 25,
	jive_i386_int_ashr = 26,
	jive_i386_int_mul_expand_signed = 27,
	jive_i386_int_mul_expand_unsigned = 28,
	
	jive_i386_int_add_immediate = 32,
	jive_i386_int_sub_immediate = 33,
	jive_i386_int_and_immediate = 34,
	jive_i386_int_or_immediate = 35,
	jive_i386_int_xor_immediate = 36,
	jive_i386_int_mul_immediate = 37,
	jive_i386_int_shl_immediate = 38,
	jive_i386_int_shr_immediate = 39,
	jive_i386_int_ashr_immediate = 40,
	
	jive_i386_int_neg = 48,
	jive_i386_int_not = 49,
	
	jive_i386_int_transfer = 56,
	
	jive_i386_call = 57,
	jive_i386_call_reg = 58,

	jive_i386_int_cmp = 60,
	jive_i386_int_cmp_immediate = 61,

	jive_i386_int_jump_sless = 70,
	jive_i386_int_jump_uless = 71,
	jive_i386_int_jump_slesseq = 72,
	jive_i386_int_jump_ulesseq = 73,
	jive_i386_int_jump_equal = 74,
	jive_i386_int_jump_notequal = 75,
	jive_i386_int_jump_sgreater = 76,
	jive_i386_int_jump_ugreater = 77,
	jive_i386_int_jump_sgreatereq = 78,
	jive_i386_int_jump_ugreatereq = 79,
	jive_i386_jump = 80,

	jive_i386_fp_load_disp = 100,

	jive_i386_sse_load32_disp = 120,
	jive_i386_sse_load_abs = 121,
	jive_i386_sse_store32_disp = 122,
} jive_i386_instruction_index;

extern const jive_instruction_class jive_i386_instructions[];
extern const struct jive_instructionset jive_i386_instructionset;

#endif
