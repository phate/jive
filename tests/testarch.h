/*
 * Copyright 2010 2011 2012 2013 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TESTARCH_H
#define JIVE_TESTARCH_H

#include <jive/arch/instruction.h>
#include <jive/arch/registers.h>
#include <jive/arch/regselector.h>
#include <jive/arch/subroutine.h>

extern const jive_register_class jive_testarch_regcls_cc;
extern const jive_register_class jive_testarch_regcls_gpr;

extern const jive_register_class jive_testarch_regcls_r0;
extern const jive_register_class jive_testarch_regcls_r1;
extern const jive_register_class jive_testarch_regcls_r2;
extern const jive_register_class jive_testarch_regcls_r3;
extern const jive_register_class jive_testarch_regcls_evenreg;
extern const jive_register_class jive_testarch_regcls_oddreg;

extern const jive_register_name jive_testarch_reg_r0;
extern const jive_register_name jive_testarch_reg_r1;
extern const jive_register_name jive_testarch_reg_r2;
extern const jive_register_name jive_testarch_reg_r3;
extern const jive_register_name jive_testarch_reg_cc;

extern const jive_instruction_class jive_testarch_instr_nop;
extern const jive_instruction_class jive_testarch_instr_add;
extern const jive_instruction_class jive_testarch_instr_load_disp;
extern const jive_instruction_class jive_testarch_instr_store_disp;

extern const jive_instruction_class jive_testarch_instr_spill_gpr;
extern const jive_instruction_class jive_testarch_instr_restore_gpr;
extern const jive_instruction_class jive_testarch_instr_move_gpr;

extern const jive_instruction_class jive_testarch_instr_setr0;
extern const jive_instruction_class jive_testarch_instr_setr1;
extern const jive_instruction_class jive_testarch_instr_setr2;
extern const jive_instruction_class jive_testarch_instr_setr3;

extern const jive_instruction_class jive_testarch_instr_add_gpr;
extern const jive_instruction_class jive_testarch_instr_sub_gpr;

extern const jive_instruction_class jive_testarch_instr_jump;

jive_subroutine_deprecated *
jive_testarch_subroutine_create(struct jive_region * region,
	size_t nparameters, const jive_argument_type parameters[],
	size_t nreturns, const jive_argument_type returns[]);

typedef struct jive_testarch_subroutine jive_testarch_subroutine;

struct jive_testarch_subroutine {
	jive_subroutine_deprecated base;
};

extern const jive_reg_classifier jive_testarch_reg_classifier;

#endif
