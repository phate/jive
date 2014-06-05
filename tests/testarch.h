/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
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
extern const jive_instruction_class jive_testarch_instr_jumpz;
extern const jive_instruction_class jive_testarch_instr_jumpnz;

jive_subroutine_deprecated *
jive_testarch_subroutine_create(struct jive_region * region,
	size_t nparameters, const jive_argument_type parameters[],
	size_t nreturns, const jive_argument_type returns[]);

jive_subroutine
jive_testarch_subroutine_begin(struct jive_graph * graph,
	size_t nparameters, const jive_argument_type parameter_types[],
	size_t nreturns, const jive_argument_type return_types[]);

typedef struct jive_testarch_subroutine jive_testarch_subroutine;

struct jive_testarch_subroutine {
	jive_subroutine_deprecated base;
};

class jive_testarch_reg_classifier final : public jive_reg_classifier {
public:
	virtual ~jive_testarch_reg_classifier() noexcept;

	virtual jive_regselect_mask
	classify_any() const override;

	virtual jive_regselect_mask
	classify_type(const jive::base::type * type, const jive_resource_class * rescls) const override;

	virtual jive_regselect_mask
	classify_fixed_unary(const jive::bits_unary_operation & op) const override;

	virtual jive_regselect_mask
	classify_fixed_binary(const jive::bits_binary_operation & op) const override;

	virtual jive_regselect_mask
	classify_fixed_compare(const jive::bits_compare_operation & op) const override;

	virtual jive_regselect_mask
	classify_float_unary(const jive::flt_unary_operation & op) const override;

	virtual jive_regselect_mask
	classify_float_binary(const jive::flt_binary_operation & op) const override;

	virtual jive_regselect_mask
	classify_float_compare(const jive::flt_compare_operation & op) const override;

	virtual jive_regselect_mask
	classify_address() const override;

	virtual size_t
	nclasses() const noexcept override;

	virtual const jive_register_class * const *
	classes() const noexcept override;
};

#endif
