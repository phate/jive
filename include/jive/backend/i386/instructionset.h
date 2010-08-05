#ifndef JIVE_BACKEND_I386_INSTRUCTIONSET_H
#define JIVE_BACKEND_I386_INSTRUCTIONSET_H

#include <jive/arch/instruction.h>

typedef enum {
	jive_i386_ret = 0,
	
	jive_i386_int_load_imm = 1,
	jive_i386_int_load32_disp = 2,
	jive_i386_int_store32_disp = 3,
	
	jive_i386_int_add = 13,
	jive_i386_int_sub = 14,
	jive_i386_int_and = 15,
	jive_i386_int_or = 16,
	jive_i386_int_xor = 17,
	jive_i386_int_mul = 18,
	
	jive_i386_int_add_immediate = 19,
	jive_i386_int_sub_immediate = 20,
	jive_i386_int_and_immediate = 21,
	jive_i386_int_or_immediate = 22,
	jive_i386_int_xor_immediate = 23,
	jive_i386_int_mul_immediate = 24,
	
	jive_i386_int_neg = 25,
	jive_i386_int_not = 26,
	
	jive_i386_int_transfer = 32
} jive_i386_instruction_index;

extern const struct jive_instruction_class jive_i386_instructions[];

#endif
