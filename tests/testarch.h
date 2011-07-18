#ifndef JIVE_TESTARCH_H
#define JIVE_TESTARCH_H

#include <jive/arch/instruction.h>
#include <jive/arch/registers.h>

typedef enum {
	cls_r0 = 0,
	cls_r1 = 1,
	cls_r2 = 2,
	cls_r3 = 3,
	cls_evenreg = 4,
	cls_oddreg = 5,
	cls_gpr = 6,
	
	cls_cc = 7
} jive_testarch_regcls_index;

extern const jive_register_class jive_testarch_regcls [];

typedef enum {
	reg_r0 = 0,
	reg_r1 = 2,
	reg_r2 = 1,
	reg_r3 = 3,
	
	reg_cc = 4
} jive_testarch_reg_index;

extern const jive_register_name jive_testarch_regs [];

typedef enum {
	nop_index = 0,
	add_index = 1
} jive_testarch_instruction_index;

extern const jive_instruction_class jive_testarch_instructions [];

#endif
