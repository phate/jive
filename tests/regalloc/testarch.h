#ifndef JIVE_TESTARCH_H
#define JIVE_TESTARCH_H

#include <jive/arch/registers.h>
#include <jive/arch/instruction.h>

typedef enum {
	cls_regs = 0,
	cls_r1 = 1,
	cls_r2 = 2,
	cls_r3 = 3,
	cls_r4 = 4
} jive_testarch_regcls_index;

extern const jive_regcls jive_testarch_regcls [];

typedef enum {
	reg_r1 = 0,
	reg_r2 = 1,
	reg_r3 = 2,
	reg_r4 = 3
} jive_testarch_reg_index;

extern const jive_cpureg jive_testarch_regs [];

typedef enum {
	instr_nop = 0,
	instr_setr1 = 1,
	instr_setr2 = 2,
	instr_setr3 = 3,
	instr_setr4 = 4,
	instr_getr1 = 1,
	instr_getr2 = 2,
	instr_getr3 = 3,
	instr_getr4 = 4,
	instr_add = 5,
	instr_sub = 6,
} jive_testarch_instruction_index;

extern const jive_instruction_class jive_testarch_instructions [];

#endif
