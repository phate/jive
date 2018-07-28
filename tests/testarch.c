/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "testarch.h"

#include <jive/common.h>

#include <jive/arch/instructionset.h>
#include <jive/arch/registers.h>
#include <jive/arch/stackslot.h>
#include <jive/arch/subroutine/nodes.h>
#include <jive/rvsdg.h>
#include <jive/rvsdg/splitnode.h>
#include <jive/types/bitstring/type.h>

const jive::registers jive_testarch_reg_r0("r0", &jive_testarch_regcls_r0, 0);
const jive::registers jive_testarch_reg_r2("r2", &jive_testarch_regcls_r2, 2);
const jive::registers jive_testarch_reg_r1("r1", &jive_testarch_regcls_r1, 1);
const jive::registers jive_testarch_reg_r3("r3", &jive_testarch_regcls_r3, 3);
const jive::registers jive_testarch_reg_cc("cc", &jive_testarch_regcls_cc, 0);

#define CLS(x) &jive_testarch_regcls_##x
#define STACK4 &jive_stackslot_class_4_4

static const jive::bittype bits16(16);
static const jive::bittype bits32(32);

const jive::register_class jive_testarch_regcls_r0(
	"r0", {&jive_testarch_reg_r0},
	&jive_testarch_regcls_evenreg, jive::resource_class::priority::reg_low,
	{{STACK4, {CLS(gpr), STACK4}}}, &bits32, 32, 0, 0);

const jive::register_class jive_testarch_regcls_r1(
	"r1", {&jive_testarch_reg_r1},
	&jive_testarch_regcls_oddreg, jive::resource_class::priority::reg_low,
	{{STACK4, {CLS(gpr), STACK4}}}, &bits32, 32, 0, 0);

const jive::register_class jive_testarch_regcls_r2(
	"r2", {&jive_testarch_reg_r2},
	&jive_testarch_regcls_evenreg, jive::resource_class::priority::reg_low,
	{{STACK4, {CLS(gpr), STACK4}}}, &bits32, 32, 0, 0);

const jive::register_class jive_testarch_regcls_r3(
	"r3", {&jive_testarch_reg_r3},
	&jive_testarch_regcls_oddreg, jive::resource_class::priority::reg_low,
	{{STACK4, {CLS(gpr), STACK4}}}, &bits32, 32, 0, 0);

const jive::register_class jive_testarch_regcls_evenreg(
	"even", {&jive_testarch_reg_r0, &jive_testarch_reg_r2},
	&jive_testarch_regcls_gpr, jive::resource_class::priority::reg_low,
	{{STACK4, {CLS(gpr), STACK4}}}, &bits32, 32, 0, 0);

const jive::register_class jive_testarch_regcls_oddreg(
	"odd", {&jive_testarch_reg_r1, &jive_testarch_reg_r3},
	&jive_testarch_regcls_gpr, jive::resource_class::priority::reg_low,
	{{STACK4, {CLS(gpr), STACK4}}}, &bits32, 32, 0, 0);

const jive::register_class jive_testarch_regcls_gpr(
	"gpr",
	{&jive_testarch_reg_r0, &jive_testarch_reg_r1, &jive_testarch_reg_r2, &jive_testarch_reg_r3},
	&jive_root_register_class, jive::resource_class::priority::reg_low,
	{{STACK4, {CLS(gpr), STACK4}}}, &bits32, 32, 0, 0);

const jive::register_class jive_testarch_regcls_cc(
	"cc", {&jive_testarch_reg_cc},
	&jive_root_register_class, jive::resource_class::priority::reg_high,
	{{CLS(gpr), {CLS(cc), CLS(gpr)}}, {STACK4, {CLS(cc), CLS(gpr), STACK4}}},
	&bits16, 32, 0, 0);

namespace jive {
namespace testarch {

#define DEFINE_TESTARCH_INSTRUCTION(NAME, INPUTS, OUTPUTS, NIMMEDIATES, FLAGS, INVERSE_JUMP) \
const instr_##NAME instr_##NAME::instance_; \
 \
instr_##NAME::instr_##NAME() \
	: instruction(#NAME, 0, #NAME, \
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
	JIVE_DEBUG_ASSERT(0); \
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
	JIVE_DEBUG_ASSERT(0); \
} \
 \
std::unique_ptr<jive::instruction> \
instr_##NAME::copy() const \
{ \
	return std::make_unique<instr_##NAME>(); \
} \

#define COMMA ,

DEFINE_TESTARCH_INSTRUCTION(nop, {}, {}, 0, jive::instruction::flags::none, nullptr)

DEFINE_TESTARCH_INSTRUCTION(
	load_disp, {&jive_testarch_regcls_gpr}, {&jive_testarch_regcls_gpr}, 1,
	jive::instruction::flags::none, nullptr)

DEFINE_TESTARCH_INSTRUCTION(
	store_disp, {&jive_testarch_regcls_gpr COMMA &jive_testarch_regcls_gpr}, {}, 1,
	jive::instruction::flags::none, nullptr)

DEFINE_TESTARCH_INSTRUCTION(
	spill_gpr, {&jive_testarch_regcls_gpr}, {}, 0,
	jive::instruction::flags::none, nullptr)

DEFINE_TESTARCH_INSTRUCTION(
	restore_gpr, {}, {&jive_testarch_regcls_gpr}, 0,
	jive::instruction::flags::none, nullptr)

DEFINE_TESTARCH_INSTRUCTION(
	move_gpr, {&jive_testarch_regcls_gpr}, {&jive_testarch_regcls_gpr}, 0,
	jive::instruction::flags::none, nullptr)

DEFINE_TESTARCH_INSTRUCTION(
	setr0, {&jive_testarch_regcls_gpr}, {&jive_testarch_regcls_r0}, 0,
	jive::instruction::flags::none, nullptr)

DEFINE_TESTARCH_INSTRUCTION(
	setr1, {&jive_testarch_regcls_gpr}, {&jive_testarch_regcls_r1}, 0,
	jive::instruction::flags::none, nullptr)

DEFINE_TESTARCH_INSTRUCTION(
	setr2, {&jive_testarch_regcls_gpr}, {&jive_testarch_regcls_r2}, 0,
	jive::instruction::flags::none, nullptr)

DEFINE_TESTARCH_INSTRUCTION(
	setr3, {&jive_testarch_regcls_gpr}, {&jive_testarch_regcls_r3}, 0,
	jive::instruction::flags::none, nullptr)

DEFINE_TESTARCH_INSTRUCTION(
	add_gpr,
	{&jive_testarch_regcls_gpr COMMA &jive_testarch_regcls_gpr}, {&jive_testarch_regcls_gpr}, 0,
	jive::instruction::flags::write_input | jive::instruction::flags::commutative, nullptr)

DEFINE_TESTARCH_INSTRUCTION(
	sub_gpr,
	{&jive_testarch_regcls_gpr COMMA &jive_testarch_regcls_gpr}, {&jive_testarch_regcls_gpr}, 0,
	jive::instruction::flags::write_input, nullptr)

DEFINE_TESTARCH_INSTRUCTION(jump, {}, {}, 0, jive::instruction::flags::none, nullptr)

DEFINE_TESTARCH_INSTRUCTION(
	jumpz, {&jive_testarch_regcls_gpr}, {}, 0,
	jive::instruction::flags::jump | jive::instruction::flags::jump_conditional_invertible,
	&instr_jumpnz::instance())

DEFINE_TESTARCH_INSTRUCTION(
	jumpnz,
	{&jive_testarch_regcls_gpr}, {}, 0,
	jive::instruction::flags::jump | jive::instruction::flags::jump_conditional_invertible,
	&instr_jumpz::instance())

DEFINE_TESTARCH_INSTRUCTION(ret, {}, {}, 0, jive::instruction::flags::none, nullptr)

}}

/* classifier */

typedef enum jive_testarch_classify_regcls {
	jive_testarch_classify_gpr = 0,
	jive_testarch_classify_cc = 1
} jive_testarch_classify_regcls;

static const std::vector<const jive::register_class*>
regclasses ({
		&jive_testarch_regcls_gpr
	, &jive_testarch_regcls_cc
});

jive_testarch_reg_classifier::~jive_testarch_reg_classifier() noexcept
{
}

jive_regselect_mask
jive_testarch_reg_classifier::classify_any() const
{
	return (1 << jive_testarch_classify_gpr) | (1 << jive_testarch_classify_cc);
}

jive_regselect_mask
jive_testarch_reg_classifier::classify_type(
	const jive::type * type, const jive::resource_class * rescls) const
{
	rescls = jive::relax(rescls);

	if (rescls == &jive_testarch_regcls_gpr) {
		return (1 << jive_testarch_classify_gpr);
	} else if (rescls == &jive_testarch_regcls_cc) {
		return (1 << jive_testarch_classify_cc);
	} else {
		return 0;
	}
}

jive_regselect_mask
jive_testarch_reg_classifier::classify_fixed_unary(const jive::bitunary_op & op) const
{
	return (1 << jive_testarch_classify_gpr);
}

jive_regselect_mask
jive_testarch_reg_classifier::classify_fixed_binary(const jive::bitbinary_op & op) const
{
	return (1 << jive_testarch_classify_gpr);
}

jive_regselect_mask
jive_testarch_reg_classifier::classify_fixed_compare(const jive::bitcompare_op & op) const
{
	return (1 << jive_testarch_classify_gpr);
}

jive_regselect_mask
jive_testarch_reg_classifier::classify_float_unary(const jive::flt::unary_op & op) const
{
	throw std::logic_error("not implemented in unit test");
}

jive_regselect_mask
jive_testarch_reg_classifier::classify_float_binary(const jive::flt::binary_op & op) const
{
	throw std::logic_error("not implemented in unit test");
}

jive_regselect_mask
jive_testarch_reg_classifier::classify_float_compare(const jive::flt::compare_op & op) const
{
	throw std::logic_error("not implemented in unit test");
}

jive_regselect_mask
jive_testarch_reg_classifier::classify_address() const
{
	return (1 << jive_testarch_classify_gpr);
}

size_t
jive_testarch_reg_classifier::nclasses() const noexcept
{
	return regclasses.size();
}

const std::vector<const jive::register_class*> &
jive_testarch_reg_classifier::classes() const noexcept
{
	return regclasses;
}

/* instructionset */

class testarch_isa final : public jive::instructionset {
public:
	virtual
	~testarch_isa()
	{}

private:
	inline constexpr
	testarch_isa()
	{}

public:
	virtual const jive::instruction *
	jump_instruction() const noexcept override
	{
		return &jive::testarch::instr_jump::instance();
	}

	virtual const jive::register_classifier *
	classifier() const noexcept override
	{
		return jive_testarch_reg_classifier::get();
	}

	virtual jive::xfer_description
	create_xfer(
		jive::region * region,
		jive::output * origin,
		const jive::resource_class * in_class,
		const jive::resource_class * out_class) override
	{
		auto in_relaxed = jive::relax(in_class);
		auto out_relaxed = jive::relax(out_class);

		if (in_relaxed == CLS(gpr) && out_relaxed == CLS(gpr)) {
			auto node = create_instruction(region, &jive::testarch::instr_move_gpr::instance(),
				{origin});
			return jive::xfer_description(node->input(0), node, node->output(0));
		}

		if (in_relaxed == CLS(gpr)) {
			auto node = create_instruction(region, &jive::testarch::instr_spill_gpr::instance(),
				{origin}, {}, {out_class->type()});
			return jive::xfer_description(node->input(0), node, node->output(0));
		}

		if (out_relaxed == CLS(gpr)) {
			auto node = create_instruction(region, &jive::testarch::instr_restore_gpr::instance(),
				{origin}, {in_class->type()}, {});
			return jive::xfer_description(node->input(0), node, node->output(0));
		}

		JIVE_ASSERT(0);
	}

	static inline const testarch_isa *
	get()
	{
		static const testarch_isa instructionset;
		return &instructionset;
	}
};

/* subroutine support */

namespace {

class testarch_c_builder_interface final : public jive::subroutine_hl_builder_interface {
public:
	virtual
	~testarch_c_builder_interface() noexcept
	{
	}

	virtual jive::output *
	value_parameter(
		jive_subroutine & subroutine,
		size_t index) override
	{
		JIVE_ASSERT(0);
		#if 0
		auto o = subroutine.builder_state->arguments[index].output;
		if (index >= 2)
			o = jive::split_op::create(o, o->port().rescls(), &jive_testarch_regcls_gpr);

		return o;
		#endif
	}

	virtual void
	value_return(
		jive_subroutine & subroutine,
		size_t index,
		jive::simple_output * value) override
	{
		JIVE_ASSERT(0);
		//subroutine.builder_state->results[index].output = value;
	}
	
	virtual jive::simple_output *
	finalize(
		jive_subroutine & subroutine) override
	{
		JIVE_ASSERT(0);
	}
};

/* subroutine abi */

class jive_testarch_subroutine_abi final : public jive::subroutine_abi {
public:
	virtual
	~jive_testarch_subroutine_abi()
	{}

private:
	inline constexpr
	jive_testarch_subroutine_abi()
	: jive::subroutine_abi()
	{}

	jive_testarch_subroutine_abi(const jive_testarch_subroutine_abi &) = delete;

	jive_testarch_subroutine_abi(jive_testarch_subroutine_abi &&) = delete;

public:
	virtual void
	prepare_stackframe(
		const jive::subroutine_op & op,
		jive::region * region,
		jive_subroutine_stackframe_info * frame,
		const jive_subroutine_late_transforms * xform) const override
	{}

	virtual jive::input *
	add_fp_dependency(
		const jive::subroutine_op & op,
		jive::region * region,
		jive::node * node) const override
	{
		JIVE_ASSERT(0);
	}

	virtual jive::input *
	add_sp_dependency(
		const jive::subroutine_op & op,
		jive::region * region,
		jive::node * node) const override
	{
		JIVE_ASSERT(0);
	}

	virtual const jive::instructionset *
	instructionset() const noexcept override
	{
		return testarch_isa::get();
	}

	static inline const jive::subroutine_abi *
	get()
	{
		static const jive_testarch_subroutine_abi abi;
		return &abi;
	}
};

}

jive_subroutine
jive_testarch_subroutine_begin(jive::graph * graph,
	size_t nparameters, const jive_argument_type parameter_types[],
	size_t nreturns, const jive_argument_type return_types[])
{
	jive::subroutine_machine_signature sig;
	
	for (size_t n = 0; n < nparameters; n++) {
		const jive::resource_class * cls;
		auto argname = jive::detail::strfmt("arg", n+1);
		switch (n) {
			case 0: cls = &jive_testarch_regcls_r1; break;
			case 1: cls = &jive_testarch_regcls_r2; break;
			default: cls = jive_fixed_stackslot_class_get(4, 4, (n - 1) * 4);
		}
		sig.arguments.emplace_back(jive::subroutine_machine_signature::argument{argname, cls, true});
	}
	
	for (size_t n = 0; n < nreturns; n++) {
		const jive::resource_class * cls;
		auto resname = jive::detail::strfmt("ret", n+1);
		switch (n) {
			case 0: cls = &jive_testarch_regcls_r1; break;
			case 1: cls = &jive_testarch_regcls_r2; break;
			default: cls = jive_fixed_stackslot_class_get(4, 4, (n - 1) * 4);
		}
		sig.results.emplace_back(jive::subroutine_machine_signature::result{resname, cls});
	}
	
	typedef jive::subroutine_machine_signature::passthrough pt;
	sig.passthroughs.emplace_back(
		pt{"mem", nullptr, false});
	sig.passthroughs.emplace_back(pt{"stackptr", &jive_testarch_regcls_r0, false});
	
	sig.abi_class = jive_testarch_subroutine_abi::get();

	std::unique_ptr<jive::subroutine_hl_builder_interface> builder(
		new testarch_c_builder_interface());
	return jive_subroutine_begin(
		graph, std::move(sig),
		std::move(builder));
}

