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

extern const jive::register_class jive_testarch_regcls_cc;
extern const jive::register_class jive_testarch_regcls_gpr;

extern const jive::register_class jive_testarch_regcls_r0;
extern const jive::register_class jive_testarch_regcls_r1;
extern const jive::register_class jive_testarch_regcls_r2;
extern const jive::register_class jive_testarch_regcls_r3;
extern const jive::register_class jive_testarch_regcls_evenreg;
extern const jive::register_class jive_testarch_regcls_oddreg;

extern const jive::registers jive_testarch_reg_r0;
extern const jive::registers jive_testarch_reg_r1;
extern const jive::registers jive_testarch_reg_r2;
extern const jive::registers jive_testarch_reg_r3;
extern const jive::registers jive_testarch_reg_cc;

namespace jive {
namespace testarch {

#define DECLARE_TESTARCH_INSTRUCTION(NAME) \
class instr_##NAME : public jive::instruction { \
public: \
	instr_##NAME(); \
\
	virtual void \
	encode(	\
		jive::section * target, \
		const jive::registers * inputs[], \
		const jive::registers * outputs[], \
		const jive_codegen_imm immediates[], \
		jive_instruction_encoding_flags * flags) const override; \
\
	virtual void \
	write_asm( \
		jive::buffer * target, \
		const jive::registers * inputs[], \
		const jive::registers * outputs[], \
		const jive_asmgen_imm immediates[], \
		jive_instruction_encoding_flags * flags) const override; \
\
	virtual std::unique_ptr<jive::instruction> \
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

DECLARE_TESTARCH_INSTRUCTION(nop);
DECLARE_TESTARCH_INSTRUCTION(load_disp);
DECLARE_TESTARCH_INSTRUCTION(store_disp);
DECLARE_TESTARCH_INSTRUCTION(spill_gpr);
DECLARE_TESTARCH_INSTRUCTION(restore_gpr);
DECLARE_TESTARCH_INSTRUCTION(move_gpr);
DECLARE_TESTARCH_INSTRUCTION(setr0);
DECLARE_TESTARCH_INSTRUCTION(setr1);
DECLARE_TESTARCH_INSTRUCTION(setr2);
DECLARE_TESTARCH_INSTRUCTION(setr3);
DECLARE_TESTARCH_INSTRUCTION(add_gpr);
DECLARE_TESTARCH_INSTRUCTION(sub_gpr);
DECLARE_TESTARCH_INSTRUCTION(jump);
DECLARE_TESTARCH_INSTRUCTION(jumpz);
DECLARE_TESTARCH_INSTRUCTION(jumpnz);
DECLARE_TESTARCH_INSTRUCTION(ret);

}}

jive_subroutine
jive_testarch_subroutine_begin(jive::graph * graph,
	size_t nparameters, const jive_argument_type parameter_types[],
	size_t nreturns, const jive_argument_type return_types[]);

class jive_testarch_reg_classifier final : public jive_reg_classifier {
public:
	virtual ~jive_testarch_reg_classifier() noexcept;

	virtual jive_regselect_mask
	classify_any() const override;

	virtual jive_regselect_mask
	classify_type(const jive::type * type, const jive::resource_class * rescls) const override;

	virtual jive_regselect_mask
	classify_fixed_unary(const jive::bits::unary_op & op) const override;

	virtual jive_regselect_mask
	classify_fixed_binary(const jive::bits::binary_op & op) const override;

	virtual jive_regselect_mask
	classify_fixed_compare(const jive::bits::compare_op & op) const override;

	virtual jive_regselect_mask
	classify_float_unary(const jive::flt::unary_op & op) const override;

	virtual jive_regselect_mask
	classify_float_binary(const jive::flt::binary_op & op) const override;

	virtual jive_regselect_mask
	classify_float_compare(const jive::flt::compare_op & op) const override;

	virtual jive_regselect_mask
	classify_address() const override;

	virtual size_t
	nclasses() const noexcept override;

	virtual const jive::register_class * const *
	classes() const noexcept override;

	static inline const jive_testarch_reg_classifier *
	get()
	{
		static const jive_testarch_reg_classifier classifier;
		return &classifier;
	}
};

#endif
