#ifndef JIVE_TESTARCH_H
#define JIVE_TESTARCH_H

#include <jive/arch/instruction.h>
#include <jive/arch/registers.h>
#include <jive/arch/regselector.h>
#include <jive/arch/subroutine.h>
#include <jive/arch/transfer-instructions.h>

typedef enum {
	cls_cc = 0,
	cls_gpr = 1,
	
	cls_r0 = 2,
	cls_r1 = 3,
	cls_r2 = 4,
	cls_r3 = 5,
	cls_evenreg = 6,
	cls_oddreg = 7
} jive_testarch_regcls_index;

extern const jive_register_class jive_testarch_regcls [];

#define jive_testarch_cls_r0 (&jive_testarch_regcls[cls_r0].base)
#define jive_testarch_cls_r1 (&jive_testarch_regcls[cls_r1].base)
#define jive_testarch_cls_r2 (&jive_testarch_regcls[cls_r2].base)
#define jive_testarch_cls_r3 (&jive_testarch_regcls[cls_r3].base)
#define jive_testarch_cls_evenreg (&jive_testarch_regcls[cls_evenreg].base)
#define jive_testarch_cls_oddreg (&jive_testarch_regcls[cls_oddreg].base)
#define jive_testarch_cls_gpr (&jive_testarch_regcls[cls_gpr].base)

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
	add_index = 1,
	load_disp_index = 2,
	store_disp_index = 3,
	
	spill_gpr_index = 4,
	restore_gpr_index = 5,
	move_gpr_index = 6,
	
	setr0_index = 7,
	setr1_index = 8,
	setr2_index = 9,
	setr3_index = 10,
	
	add_gpr_index = 11,
	sub_gpr_index = 12,
} jive_testarch_instruction_index;

extern const jive_instruction_class jive_testarch_instructions [];

extern const struct jive_transfer_instructions_factory jive_testarch_xfer_factory;

jive_subroutine *
jive_testarch_subroutine_create(struct jive_region * region,
	size_t nparameters, const jive_argument_type parameters[],
	size_t nreturns, const jive_argument_type returns[]);

typedef struct jive_testarch_subroutine jive_testarch_subroutine;

struct jive_testarch_subroutine {
	jive_subroutine base;
	
	jive_subroutine_passthrough stackptr;
};

extern const jive_reg_classifier jive_testarch_reg_classifier;

#endif
