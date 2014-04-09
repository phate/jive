/*
 * Copyright 2010 2011 2012 2013 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "testarch.h"

#include <stdio.h>

#include <jive/common.h>

#include <jive/arch/instructionset.h>
#include <jive/arch/registers.h>
#include <jive/arch/stackslot.h>
#include <jive/arch/subroutine-private.h>
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

static const jive_bitstring_type bits16(16);
static const jive_bitstring_type bits32(32);
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

static const jive_register_class * gpr_params[] = {
	&jive_testarch_regcls_gpr,
	&jive_testarch_regcls_gpr
};

static const jive_register_class * special_params[] = {
	&jive_testarch_regcls_r0,
	&jive_testarch_regcls_r1,
	&jive_testarch_regcls_r2,
	&jive_testarch_regcls_r3
};

const jive_instruction_class jive_testarch_instr_nop = {
	name : "nop",
	mnemonic : "nop",
	encode : 0,
	write_asm : 0,
	inregs : 0, outregs : 0,
	flags : jive_instruction_flags_none,
	ninputs : 0, noutputs : 0, nimmediates : 0,
	code : 0
};

const jive_instruction_class jive_testarch_instr_add = {
	name : "add",
	mnemonic : "add",
	encode : 0,
	write_asm : 0,
	inregs : gpr_params, outregs : gpr_params,
	flags : jive_instruction_write_input | jive_instruction_commutative,
	ninputs : 2, noutputs : 1, nimmediates : 0,
	code : 0
};

const jive_instruction_class jive_testarch_instr_load_disp = {
	name : "load_disp",
	mnemonic : "load_disp",
	encode : 0,
	write_asm : 0,
	inregs : gpr_params, outregs : gpr_params,
	flags : jive_instruction_flags_none,
	ninputs : 1, noutputs : 1, nimmediates : 1,
	code : 0
};

const jive_instruction_class jive_testarch_instr_store_disp = {
	name : "store_disp",
	mnemonic : "load_disp",
	encode : 0,
	write_asm : 0,
	inregs : gpr_params, outregs : 0, flags : jive_instruction_flags_none,
	ninputs : 2, noutputs : 0, nimmediates : 1,
	code : 0
};

const jive_instruction_class jive_testarch_instr_spill_gpr = {
	name : "spill_gpr",
	mnemonic : "spill_gpr",
	encode : 0,
	write_asm : 0,
	inregs : gpr_params, outregs : 0, flags : jive_instruction_flags_none,
	ninputs : 1, noutputs : 0, nimmediates : 0,
	code : 0
};

const jive_instruction_class jive_testarch_instr_restore_gpr = {
	name : "restore_gpr",
	mnemonic : "restore_gpr",
	encode : 0,
	write_asm : 0,
	inregs : 0, outregs : gpr_params, flags : jive_instruction_flags_none,
	ninputs : 0, noutputs : 1, nimmediates : 0,
	code : 0
};

const jive_instruction_class jive_testarch_instr_move_gpr = {
	name : "move_gpr",
	mnemonic : "move_gpr",
	encode : 0,
	write_asm : 0,
	inregs : gpr_params, outregs : gpr_params, flags : jive_instruction_flags_none,
	ninputs : 1, noutputs : 1, nimmediates : 0,
	code : 0
};

const jive_instruction_class jive_testarch_instr_setr0 = {
	name : "setr0",
	mnemonic : "setr0",
	encode : 0,
	write_asm : 0,
	inregs : gpr_params, outregs : &special_params[0], flags : jive_instruction_flags_none,
	ninputs : 1, noutputs : 1, nimmediates : 0,
	code : 0
};
const jive_instruction_class jive_testarch_instr_setr1 = {
	name : "setr1",
	mnemonic : "setr1",
	encode : 0,
	write_asm : 0,
	inregs : gpr_params, outregs : &special_params[1], flags : jive_instruction_flags_none,
	ninputs : 1, noutputs : 1, nimmediates : 0,
	code : 0
};
const jive_instruction_class jive_testarch_instr_setr2 = {
	name : "setr2",
	mnemonic : "setr2",
	encode : 0,
	write_asm : 0,
	inregs : gpr_params, outregs : &special_params[2], flags : jive_instruction_flags_none,
	ninputs : 1, noutputs : 1, nimmediates : 0,
	code : 0
};
const jive_instruction_class jive_testarch_instr_setr3 = {
	name : "setr3",
	mnemonic : "setr3",
	encode : 0,
	write_asm : 0,
	inregs : gpr_params, outregs : &special_params[3], flags : jive_instruction_flags_none,
	ninputs : 1, noutputs : 1, nimmediates : 0,
	code : 0
};

const jive_instruction_class jive_testarch_instr_add_gpr = {
	name : "add_gpr",
	mnemonic : "add_gpr",
	encode : 0,
	write_asm : 0,
	inregs : gpr_params, outregs : gpr_params,
	flags : jive_instruction_write_input | jive_instruction_commutative,
	ninputs : 2, noutputs : 1, nimmediates : 0,
	code : 0
};
const jive_instruction_class jive_testarch_instr_sub_gpr = {
	name : "sub_gpr",
	mnemonic : "sub_gpr",
	encode : 0,
	write_asm : 0,
	inregs : gpr_params, outregs : gpr_params, flags : jive_instruction_write_input,
	ninputs : 2, noutputs : 1, nimmediates : 0,
	code : 0
};
const jive_instruction_class jive_testarch_instr_jump = {
	name : "jump",
	mnemonic : "jump",
	encode : 0,
	write_asm : 0,
	inregs : 0, outregs : 0, flags : jive_instruction_flags_none,
	ninputs : 0, noutputs : 0, nimmediates : 0,
	code : 0
};
const jive_instruction_class jive_testarch_instr_jumpz = {
	name : "jumpz",
	mnemonic : "jumpz",
	encode : 0,
	write_asm : 0,
	inregs : gpr_params, outregs : 0, flags : jive_instruction_jump | jive_instruction_jump_conditional_invertible,
	ninputs : 1, noutputs : 0, nimmediates : 0,
	code : 0,
	inverse_jump : &jive_testarch_instr_jumpnz
};
const jive_instruction_class jive_testarch_instr_jumpnz = {
	name : "jumpnz",
	mnemonic : "jumpnz",
	encode : 0,
	write_asm : 0,
	inregs : gpr_params, outregs : 0, flags : jive_instruction_jump | jive_instruction_jump_conditional_invertible,
	ninputs : 1, noutputs : 0, nimmediates : 0,
	code : 0,
	inverse_jump : &jive_testarch_instr_jumpz
};

static jive_xfer_description
create_xfer(jive_region * region, jive_output * origin,
	const jive_resource_class * in_class, const jive_resource_class * out_class)
{
	jive_xfer_description xfer;
	
	const jive_resource_class * in_relaxed = jive_resource_class_relax(in_class);
	const jive_resource_class * out_relaxed = jive_resource_class_relax(out_class);
	
	if (in_relaxed == CLS(gpr) && out_relaxed == CLS(gpr)) {
		jive_output * tmparray8[] = {origin};
		xfer.node = jive_instruction_node_create(
			region,
			&jive_testarch_instr_move_gpr,
			tmparray8, NULL);
		xfer.input = xfer.node->inputs[0];
		xfer.output = xfer.node->outputs[0];
	} else if (in_relaxed == CLS(gpr)) {
		jive_output * tmparray9[] = {origin};
		xfer.node = jive_instruction_node_create(
			region,
			&jive_testarch_instr_spill_gpr,
			tmparray9, NULL);
		xfer.input = xfer.node->inputs[0];
		xfer.output = jive_node_add_output(xfer.node, jive_resource_class_get_type(out_class));
	} else if (out_relaxed == CLS(gpr)) {
		xfer.node = jive_instruction_node_create(
			region,
			&jive_testarch_instr_restore_gpr,
			NULL, NULL);
		xfer.input = jive_node_add_input(xfer.node, jive_resource_class_get_type(in_class), origin);
		xfer.output = xfer.node->outputs[0];
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

static jive_regselect_mask
jive_testarch_classify_type_(const jive_type * type, const jive_resource_class * rescls)
{
	rescls = jive_resource_class_relax(rescls);
	
	if (rescls == &jive_testarch_regcls_gpr.base)
		return (1 << jive_testarch_classify_gpr);
	else if (rescls == &jive_testarch_regcls_cc.base)
		return (1 << jive_testarch_classify_cc);
	
	return 0;
}

static jive_regselect_mask
jive_testarch_classify_fixed_arithmetic_(jive_bitop_code op, size_t nbits)
{
	return (1 << jive_testarch_classify_gpr);
}

static jive_regselect_mask
jive_testarch_classify_fixed_compare_(jive_bitcmp_code op, size_t nbits)
{
	return (1 << jive_testarch_classify_gpr);
}

static jive_regselect_mask
jive_testarch_classify_address_(void)
{
	return (1 << jive_testarch_classify_gpr);
}

static const jive_register_class * classes [] = {
	[jive_testarch_classify_gpr] = &jive_testarch_regcls_gpr,
	[jive_testarch_classify_cc] = &jive_testarch_regcls_cc,
};

const jive_reg_classifier jive_testarch_reg_classifier = {
	any : (1 << jive_testarch_classify_gpr) | (1 << jive_testarch_classify_cc),
	classify_type : jive_testarch_classify_type_,
	classify_fixed_arithmetic : jive_testarch_classify_fixed_arithmetic_,
	classify_float_arithmetic : NULL,
	classify_fixed_compare : jive_testarch_classify_fixed_compare_,
	classify_float_compare : NULL,
	classify_address : jive_testarch_classify_address_,
	
	nclasses : 2,
	classes : classes,
};

/* tie it all together */

const jive_instructionset_class testarch_isa_class = {
	create_xfer : create_xfer,
};

const jive_instructionset testarch_isa = {
	class_ : &testarch_isa_class,
	jump_instruction_class : &jive_testarch_instr_jump,
	reg_classifier : &jive_testarch_reg_classifier
};

/* subroutine support */

static jive_output *
jive_testarch_subroutine_value_parameter_(jive_subroutine_deprecated * self_, size_t index)
{
	jive_testarch_subroutine * self = (jive_testarch_subroutine *) self_;
	jive_gate * gate = self->base.parameters[index];
	jive_output * output = jive_node_gate_output(self->base.enter, gate);
	if (index >= 2) {
		const jive_type * in_type = jive_gate_get_type(gate);
		const jive_type * out_type =
			jive_resource_class_get_type(&jive_testarch_regcls_gpr.base);
		jive_node * node = jive_splitnode_create(self->base.enter->region,
			in_type, output, gate->required_rescls,
			out_type, &jive_testarch_regcls_gpr.base);
		output = node->outputs[0];
	}
	return output;
}

static jive_input *
jive_testarch_subroutine_value_return_(jive_subroutine_deprecated * self_, size_t index, jive_output * value)
{
	jive_testarch_subroutine * self = (jive_testarch_subroutine *) self_;
	jive_gate * gate = self->base.returns[index];
	return jive_node_gate_input(self->base.leave, gate, value);
}

extern const jive_subroutine_class JIVE_TESTARCH_SUBROUTINE;

static void
jive_testarch_subroutine_prepare_stackframe_(
	jive_subroutine_deprecated * self,
	const jive_subroutine_late_transforms * xfrm)
{
}

const jive_subroutine_class JIVE_TESTARCH_SUBROUTINE = {
	fini : jive_subroutine_fini_,
	value_parameter : jive_testarch_subroutine_value_parameter_,
	value_return : jive_testarch_subroutine_value_return_,
};

static const jive_subroutine_abi_class JIVE_TESTARCH_SUBROUTINE_ABI = {
	prepare_stackframe : jive_testarch_subroutine_prepare_stackframe_,
	add_fp_dependency : NULL,
	add_sp_dependency : NULL,
	instructionset : &testarch_isa
};

jive_subroutine_deprecated *
jive_testarch_subroutine_create(jive_region * region,
	size_t nparameters, const jive_argument_type parameter_types[],
	size_t nreturns, const jive_argument_type return_types[])
{
	jive_graph * graph = region->graph;
	jive_context * context = graph->context;
	jive_testarch_subroutine * self = jive_context_malloc(context, sizeof(*self));
	jive_subroutine_init_(&self->base, &JIVE_TESTARCH_SUBROUTINE, context,
		nparameters, parameter_types, nreturns, return_types, 2);
	self->base.abi_class = &JIVE_TESTARCH_SUBROUTINE_ABI;
	
	size_t n;
	
	for (n = 0; n < nparameters; n++) {
		char argname[80];
		snprintf(argname, sizeof(argname), "arg%zd", n + 1);
		const jive_resource_class * cls;
		switch (n) {
			case 0: cls = &jive_testarch_regcls_r1.base; break;
			case 1: cls = &jive_testarch_regcls_r2.base; break;
			default: cls = jive_fixed_stackslot_class_get(4, 4, (n - 1) * 4);
		}
		self->base.parameters[n] = jive_resource_class_create_gate(cls, graph, argname);
	}
	
	for (n = 0; n < nreturns; n++) {
		char argname[80];
		snprintf(argname, sizeof(argname), "ret%zd", n + 1);
		const jive_resource_class * cls;
		switch (n) {
			case 0: cls = &jive_testarch_regcls_r1.base; break;
			case 1: cls = &jive_testarch_regcls_r2.base; break;
			default: cls = jive_fixed_stackslot_class_get(4, 4, (n - 1) * 4);
		}
		self->base.returns[n] = jive_resource_class_create_gate(cls, graph, argname);
	}
	
	jive_subroutine_create_region_and_nodes(&self->base, region);
	
	self->base.passthroughs[0] = jive_subroutine_create_passthrough_memorystate(
		&self->base, "mem");
	self->base.passthroughs[1] = jive_subroutine_create_passthrough(
		&self->base, &jive_testarch_regcls_r0.base, "stackptr");
	self->base.passthroughs[1].gate->may_spill = false;
	
	return &self->base;
}

jive_subroutine
jive_testarch_subroutine_begin(jive_graph * graph,
	size_t nparameters, const jive_argument_type parameter_types[],
	size_t nreturns, const jive_argument_type return_types[])
{
	jive_subroutine_deprecated * s = jive_testarch_subroutine_create(
		graph->root_region,
		nparameters, parameter_types,
		nreturns, return_types);
	
	jive_subroutine sub = {
		region : s->region,
		old_subroutine_struct : s
	};
	
	s->region->bottom = NULL;
	
	return sub;
}

