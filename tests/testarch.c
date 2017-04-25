/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "testarch.h"

#include <stdio.h>

#include <jive/common.h>

#include <jive/arch/instructionset.h>
#include <jive/arch/registers.h>
#include <jive/arch/stackslot.h>
#include <jive/arch/subroutine/nodes.h>
#include <jive/types/bitstring/type.h>
#include <jive/vsdg.h>
#include <jive/vsdg/splitnode.h>

const jive_register_name jive_testarch_reg_r0 = {
	base : {name : "r0", resource_class : &jive_testarch_regcls_r0.base },
	code : 0
};
const jive_register_name jive_testarch_reg_r2 = {
	base : {name : "r2", resource_class : &jive_testarch_regcls_r2.base },
	code : 2
};
const jive_register_name jive_testarch_reg_r1 = {
	base : {name : "r1", resource_class : &jive_testarch_regcls_r1.base },
	code : 1
};
const jive_register_name jive_testarch_reg_r3 = {
	base : {name : "r3", resource_class : &jive_testarch_regcls_r3.base },
	code : 3
};
const jive_register_name jive_testarch_reg_cc = {
	base : {name : "cc", resource_class : &jive_testarch_regcls_cc.base },
	code : 0
};

static const jive_resource_name * jive_testarch_regcls_r0_names [] = {
	&jive_testarch_reg_r0.base
};
static const jive_resource_name * jive_testarch_regcls_r1_names [] = {
	&jive_testarch_reg_r1.base
};
static const jive_resource_name * jive_testarch_regcls_r2_names [] = {
	&jive_testarch_reg_r2.base
};
static const jive_resource_name * jive_testarch_regcls_r3_names [] = {
	&jive_testarch_reg_r3.base
};

static const jive_resource_name * jive_testarch_regcls_evenreg_names [] = {
	&jive_testarch_reg_r0.base,
	&jive_testarch_reg_r2.base
};
static const jive_resource_name * jive_testarch_regcls_oddreg_names [] = {
	&jive_testarch_reg_r1.base,
	&jive_testarch_reg_r3.base
};
static const jive_resource_name * jive_testarch_regcls_gpr_names [] = {
	&jive_testarch_reg_r0.base,
	&jive_testarch_reg_r1.base,
	&jive_testarch_reg_r2.base,
	&jive_testarch_reg_r3.base
};
static const jive_resource_name * jive_testarch_regcls_cc_names [] = {
	&jive_testarch_reg_cc.base,
};

#define CLS(x) &jive_testarch_regcls_##x.base
#define STACK4 &jive_stackslot_class_4_4.base
#define VIA (const jive_resource_class * const[])

static const jive::bits::type bits16(16);
static const jive::bits::type bits32(32);
const jive_resource_class_demotion  tmparray0[] = {
			{STACK4, VIA {CLS(gpr), STACK4, NULL}},
			{NULL, NULL}
		};

const jive_register_class jive_testarch_regcls_r0 = {
	base : {
		class_ : &JIVE_REGISTER_RESOURCE,
		name : "r0",
		limit : 1, names : jive_testarch_regcls_r0_names,
		parent : &jive_testarch_regcls_evenreg.base, depth : 4,
		priority : jive_resource_class_priority_reg_low,
		demotions : tmparray0,
		type : &bits32
	},
	nbits : 32
};
const jive_resource_class_demotion  tmparray1[] = {
			{STACK4, VIA {CLS(gpr), STACK4, NULL}},
			{NULL, NULL}
		};
const jive_register_class jive_testarch_regcls_r1 = {
	base : {
		class_ : &JIVE_REGISTER_RESOURCE,
		name : "r1",
		limit : 1, names : jive_testarch_regcls_r1_names,
		parent : &jive_testarch_regcls_oddreg.base, depth : 4,
		priority : jive_resource_class_priority_reg_low,
		demotions : tmparray1,
		type : &bits32
	},
	nbits : 32
};
const jive_resource_class_demotion  tmparray2[] = {
			{STACK4, VIA {CLS(gpr), STACK4, NULL}},
			{NULL, NULL}
		};
const jive_register_class jive_testarch_regcls_r2 = {
	base : {
		class_ : &JIVE_REGISTER_RESOURCE,
		name : "r2",
		limit : 1, names : jive_testarch_regcls_r2_names,
		parent : &jive_testarch_regcls_evenreg.base, depth : 4,
		priority : jive_resource_class_priority_reg_low,
		demotions : tmparray2,
		type : &bits32
	},
	nbits : 32
};
const jive_resource_class_demotion  tmparray3[] = {
			{STACK4, VIA {CLS(gpr), STACK4, NULL}},
			{NULL, NULL}
		};
const jive_register_class jive_testarch_regcls_r3 = {
	base : {
		class_ : &JIVE_REGISTER_RESOURCE,
		name : "r3",
		limit : 1, names : jive_testarch_regcls_r3_names,
		parent : &jive_testarch_regcls_oddreg.base, depth : 4,
		priority : jive_resource_class_priority_reg_low,
		demotions : tmparray3,
		type : &bits32
	},
	nbits : 32
};
const jive_resource_class_demotion  tmparray4[] = {
			{STACK4, VIA {CLS(gpr), STACK4, NULL}},
			{NULL, NULL}
		};
const jive_register_class jive_testarch_regcls_evenreg = {
	base : {
		class_ : &JIVE_REGISTER_RESOURCE,
		name : "even",
		limit : 2, names : jive_testarch_regcls_evenreg_names,
		parent : &jive_testarch_regcls_gpr.base, depth : 3,
		priority : jive_resource_class_priority_reg_low,
		demotions : tmparray4,
		type : &bits32
	},
	nbits : 32
};
const jive_resource_class_demotion  tmparray5[] = {
			{STACK4, VIA {CLS(gpr), STACK4, NULL}},
			{NULL, NULL}
		};
const jive_register_class jive_testarch_regcls_oddreg = {
	base : {
		class_ : &JIVE_REGISTER_RESOURCE,
		name : "odd",
		limit : 2, names : jive_testarch_regcls_oddreg_names,
		parent : &jive_testarch_regcls_gpr.base, depth : 3,
		priority : jive_resource_class_priority_reg_low,
		demotions : tmparray5,
		type : &bits32
	},
	nbits : 32
};
const jive_resource_class_demotion  tmparray6[] = {
			{STACK4, VIA {CLS(gpr), STACK4, NULL}},
			{NULL, NULL}
		};
const jive_register_class jive_testarch_regcls_gpr = {
	base : {
		class_ : &JIVE_REGISTER_RESOURCE,
		name : "gpr",
		limit : 4, names : jive_testarch_regcls_gpr_names,
		parent : &jive_root_register_class, depth : 2,
		priority : jive_resource_class_priority_reg_low,
		demotions : tmparray6,
		type : &bits32
	},
	nbits : 32
};
const jive_resource_class_demotion  tmparray7[] = {
			{CLS(gpr), VIA {CLS(cc), CLS(gpr), NULL}},
			{STACK4, VIA {CLS(cc), CLS(gpr), STACK4, NULL}},
			{NULL, NULL}
		};
const jive_register_class jive_testarch_regcls_cc = {
	base : {
		class_ : &JIVE_REGISTER_RESOURCE,
		name : "cc",
		limit : 1, names : jive_testarch_regcls_cc_names,
		parent : &jive_root_register_class, depth : 2,
		priority : jive_resource_class_priority_reg_high,
		demotions : tmparray7,
		type : &bits16
	},
	nbits : 32
};

namespace jive {
namespace testarch {

#define DEFINE_TESTARCH_INSTRUCTION(NAME, INPUTS, OUTPUTS, NIMMEDIATES, FLAGS, INVERSE_JUMP) \
const instr_##NAME instr_##NAME::instance_; \
 \
instr_##NAME::instr_##NAME() \
	: instruction_class(#NAME, 0, #NAME, \
		INPUTS, OUTPUTS, NIMMEDIATES, \
		FLAGS, INVERSE_JUMP) \
	{} \
\
void \
instr_##NAME::encode( \
	struct jive_section * target, \
	const jive_register_name * inputs[], \
	const jive_register_name * outputs[], \
	const jive_codegen_imm immediates[], \
	jive_instruction_encoding_flags * flags) const \
{ \
	JIVE_DEBUG_ASSERT(0); \
} \
 \
void \
instr_##NAME::write_asm( \
	struct jive_buffer * target, \
	const jive_register_name * inputs[], \
	const jive_register_name * outputs[], \
	const jive_asmgen_imm immediates[], \
	jive_instruction_encoding_flags * flags) const \
{ \
	JIVE_DEBUG_ASSERT(0); \
} \
 \
std::unique_ptr<jive::instruction_class> \
instr_##NAME::copy() const \
{ \
	return std::make_unique<instr_##NAME>(); \
} \

#define COMMA ,

DEFINE_TESTARCH_INSTRUCTION(nop, {}, {}, 0, jive_instruction_flags_none, nullptr);

DEFINE_TESTARCH_INSTRUCTION(
	load_disp,
	{&jive_testarch_regcls_gpr}, {&jive_testarch_regcls_gpr}, 1,
	jive_instruction_flags_none, nullptr);

DEFINE_TESTARCH_INSTRUCTION(
	store_disp,
	{&jive_testarch_regcls_gpr COMMA &jive_testarch_regcls_gpr}, {}, 1,
	jive_instruction_flags_none, nullptr);

DEFINE_TESTARCH_INSTRUCTION(
	spill_gpr,
	{&jive_testarch_regcls_gpr}, {}, 0,
	jive_instruction_flags_none, nullptr);

DEFINE_TESTARCH_INSTRUCTION(
	restore_gpr,
	{}, {&jive_testarch_regcls_gpr}, 0,
	jive_instruction_flags_none, nullptr);

DEFINE_TESTARCH_INSTRUCTION(
	move_gpr,
	{&jive_testarch_regcls_gpr}, {&jive_testarch_regcls_gpr}, 0,
	jive_instruction_flags_none, nullptr);

DEFINE_TESTARCH_INSTRUCTION(
	setr0,
	{&jive_testarch_regcls_gpr}, {&jive_testarch_regcls_r0}, 0,
	jive_instruction_flags_none, nullptr);

DEFINE_TESTARCH_INSTRUCTION(
	setr1,
	{&jive_testarch_regcls_gpr}, {&jive_testarch_regcls_r1}, 0,
	jive_instruction_flags_none, nullptr);

DEFINE_TESTARCH_INSTRUCTION(
	setr2,
	{&jive_testarch_regcls_gpr}, {&jive_testarch_regcls_r2}, 0,
	jive_instruction_flags_none, nullptr);

DEFINE_TESTARCH_INSTRUCTION(
	setr3,
	{&jive_testarch_regcls_gpr}, {&jive_testarch_regcls_r3}, 0,
	jive_instruction_flags_none, nullptr);

DEFINE_TESTARCH_INSTRUCTION(
	add_gpr,
	{&jive_testarch_regcls_gpr COMMA &jive_testarch_regcls_gpr}, {&jive_testarch_regcls_gpr}, 0,
	jive_instruction_write_input | jive_instruction_commutative, nullptr);

DEFINE_TESTARCH_INSTRUCTION(
	sub_gpr,
	{&jive_testarch_regcls_gpr COMMA &jive_testarch_regcls_gpr}, {&jive_testarch_regcls_gpr}, 0,
	jive_instruction_write_input, nullptr);

DEFINE_TESTARCH_INSTRUCTION(jump, {}, {}, 0, jive_instruction_flags_none, nullptr);

DEFINE_TESTARCH_INSTRUCTION(
	jumpz,
	{&jive_testarch_regcls_gpr}, {}, 0,
	jive_instruction_jump | jive_instruction_jump_conditional_invertible,
	&instr_jumpnz::instance());

DEFINE_TESTARCH_INSTRUCTION(
	jumpnz,
	{&jive_testarch_regcls_gpr}, {}, 0,
	jive_instruction_jump | jive_instruction_jump_conditional_invertible,
	&instr_jumpz::instance());

DEFINE_TESTARCH_INSTRUCTION(ret, {}, {}, 0, jive_instruction_flags_none, nullptr);

}}

static jive_xfer_description
create_xfer(jive::region * region, jive::output * origin,
	const jive_resource_class * in_class, const jive_resource_class * out_class)
{
	jive_xfer_description xfer;
	
	const jive_resource_class * in_relaxed = jive_resource_class_relax(in_class);
	const jive_resource_class * out_relaxed = jive_resource_class_relax(out_class);
	
	if (in_relaxed == CLS(gpr) && out_relaxed == CLS(gpr)) {
		jive::oport * tmparray8[] = {origin};
		xfer.node = jive_instruction_node_create(
			region,
			&jive::testarch::instr_move_gpr::instance(),
			tmparray8, NULL);
		xfer.input = dynamic_cast<jive::input*>(xfer.node->input(0));
		xfer.output = dynamic_cast<jive::output*>(xfer.node->output(0));
	} else if (in_relaxed == CLS(gpr)) {
		jive::oport * tmparray9[] = {origin};
		xfer.node = jive_instruction_node_create(
			region,
			&jive::testarch::instr_spill_gpr::instance(),
			tmparray9, NULL);
		xfer.input = dynamic_cast<jive::input*>(xfer.node->input(0));
		xfer.output = dynamic_cast<jive::output*>(xfer.node->add_output(
			jive_resource_class_get_type(out_class)));
	} else if (out_relaxed == CLS(gpr)) {
		xfer.node = jive_instruction_node_create(
			region,
			&jive::testarch::instr_restore_gpr::instance(),
			NULL, NULL);
		xfer.input = dynamic_cast<jive::input*>(xfer.node->add_input(
			jive_resource_class_get_type(in_class), origin));
		xfer.output = dynamic_cast<jive::output*>(xfer.node->output(0));
	} else {
		JIVE_DEBUG_ASSERT(false);
	}
	
	return xfer;
}

/* classifier */

typedef enum jive_testarch_classify_regcls {
	jive_testarch_classify_gpr = 0,
	jive_testarch_classify_cc = 1
} jive_testarch_classify_regcls;

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
	const jive::base::type * type, const jive_resource_class * rescls) const
{
	rescls = jive_resource_class_relax(rescls);
	
	if (rescls == &jive_testarch_regcls_gpr.base) {
		return (1 << jive_testarch_classify_gpr);
	} else if (rescls == &jive_testarch_regcls_cc.base) {
		return (1 << jive_testarch_classify_cc);
	} else {
		return 0;
	}
}

jive_regselect_mask
jive_testarch_reg_classifier::classify_fixed_unary(const jive::bits::unary_op & op) const
{
	return (1 << jive_testarch_classify_gpr);
}

jive_regselect_mask
jive_testarch_reg_classifier::classify_fixed_binary(const jive::bits::binary_op & op) const
{
	return (1 << jive_testarch_classify_gpr);
}

jive_regselect_mask
jive_testarch_reg_classifier::classify_fixed_compare(const jive::bits::compare_op & op) const
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
	return 2;
}

const jive_register_class * const *
jive_testarch_reg_classifier::classes() const noexcept
{
	static const jive_register_class * classes [] = {
		[jive_testarch_classify_gpr] = &jive_testarch_regcls_gpr,
		[jive_testarch_classify_cc] = &jive_testarch_regcls_cc,
	};
	return classes;
}

static const jive_testarch_reg_classifier classifier;

/* tie it all together */

const jive_instructionset_class testarch_isa_class = {
	create_xfer : create_xfer,
};

const jive_instructionset testarch_isa = {
	class_ : &testarch_isa_class,
	jump_instruction_class : &jive::testarch::instr_jump::instance(),
	reg_classifier : &classifier
};

/* subroutine support */

namespace {

class testarch_c_builder_interface final : public jive::subroutine_hl_builder_interface {
public:
	virtual
	~testarch_c_builder_interface() noexcept
	{
	}

	virtual jive::oport *
	value_parameter(
		jive_subroutine & subroutine,
		size_t index) override
	{
		jive::oport * o = subroutine.builder_state->arguments[index].output;
	
		if (index >= 2) {
			auto node = jive_splitnode_create(subroutine.region, o, o->gate()->rescls(),
				&jive_testarch_regcls_gpr.base);
			o = node->output(0);
		}
		return o;
	}

	virtual void
	value_return(
		jive_subroutine & subroutine,
		size_t index,
		jive::output * value) override
	{
		subroutine.builder_state->results[index].output = value;
	}
	
	virtual jive::output *
	finalize(
		jive_subroutine & subroutine) override
	{
		jive::node * ret_instr = jive_instruction_node_create(subroutine.region,
			&jive::testarch::instr_ret::instance(), {}, {}, {}, {}, {&jive::ctl::boolean});
		ret_instr->add_input(subroutine.builder_state->passthroughs[1].gate,
			subroutine.builder_state->passthroughs[1].output);
		return dynamic_cast<jive::output*>(ret_instr->output(0));
	}
};

}

static void
jive_testarch_subroutine_prepare_stackframe_(
	const jive::subroutine_op & op,
	jive::region * region,
	jive_subroutine_stackframe_info * frame,
	const jive_subroutine_late_transforms * xfrm)
{
}

static const jive_subroutine_abi_class JIVE_TESTARCH_SUBROUTINE_ABI = {
	prepare_stackframe : jive_testarch_subroutine_prepare_stackframe_,
	add_fp_dependency : NULL,
	add_sp_dependency : NULL,
	instructionset : &testarch_isa
};

jive_subroutine
jive_testarch_subroutine_begin(jive::graph * graph,
	size_t nparameters, const jive_argument_type parameter_types[],
	size_t nreturns, const jive_argument_type return_types[])
{
	jive::subroutine_machine_signature sig;
	
	for (size_t n = 0; n < nparameters; n++) {
		char argname[80];
		snprintf(argname, sizeof(argname), "arg%zd", n + 1);
		const jive_resource_class * cls;
		switch (n) {
			case 0: cls = &jive_testarch_regcls_r1.base; break;
			case 1: cls = &jive_testarch_regcls_r2.base; break;
			default: cls = jive_fixed_stackslot_class_get(4, 4, (n - 1) * 4);
		}
		sig.arguments.emplace_back(jive::subroutine_machine_signature::argument{argname, cls, true});
	}
	
	for (size_t n = 0; n < nreturns; n++) {
		char resname[80];
		snprintf(resname, sizeof(resname), "ret%zd", n + 1);
		const jive_resource_class * cls;
		switch (n) {
			case 0: cls = &jive_testarch_regcls_r1.base; break;
			case 1: cls = &jive_testarch_regcls_r2.base; break;
			default: cls = jive_fixed_stackslot_class_get(4, 4, (n - 1) * 4);
		}
		sig.results.emplace_back(jive::subroutine_machine_signature::result{resname, cls});
	}
	
	typedef jive::subroutine_machine_signature::passthrough pt;
	sig.passthroughs.emplace_back(
		pt{"mem", nullptr, false});
	sig.passthroughs.emplace_back(
		pt{"stackptr", &jive_testarch_regcls_r0.base, false});
	
	sig.abi_class = &JIVE_TESTARCH_SUBROUTINE_ABI;
	
	std::unique_ptr<jive::subroutine_hl_builder_interface> builder(
		new testarch_c_builder_interface());
	return jive_subroutine_begin(
		graph, std::move(sig),
		std::move(builder));
}

