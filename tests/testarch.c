#include "testarch.h"

#include <stdio.h>

#include <jive/common.h>

#include <jive/arch/registers.h>
#include <jive/arch/stackslot.h>
#include <jive/arch/subroutine-private.h>
#include <jive/bitstring/type.h>
#include <jive/regalloc/auxnodes.h>
#include <jive/vsdg.h>

const jive_register_name jive_testarch_regs [] = {
	[reg_r0] = {.base = {.name = "r0", .resource_class = &jive_testarch_regcls[cls_r0].base }, .code = 0},
	[reg_r2] = {.base = {.name = "r2", .resource_class = &jive_testarch_regcls[cls_r2].base }, .code = 2},
	[reg_r1] = {.base = {.name = "r1", .resource_class = &jive_testarch_regcls[cls_r1].base }, .code = 1},
	[reg_r3] = {.base = {.name = "r3", .resource_class = &jive_testarch_regcls[cls_r3].base }, .code = 3},
	
	[reg_cc] = {.base = {.name = "cc", .resource_class = &jive_testarch_regcls[cls_cc].base }, .code = 0},
};

static const jive_resource_name * allnames [] = {
	&jive_testarch_regs[reg_r0].base,
	&jive_testarch_regs[reg_r2].base,
	&jive_testarch_regs[reg_r1].base,
	&jive_testarch_regs[reg_r3].base,
	&jive_testarch_regs[reg_cc].base,
};

static const jive_bitstring_type bits16 = {{{&JIVE_BITSTRING_TYPE}}, 16};
static const jive_bitstring_type bits32 = {{{&JIVE_BITSTRING_TYPE}}, 32};

#define CLS(x) &jive_testarch_regcls[cls_##x].base
#define STACK32 &jive_stackslot_class_32_32.base
#define VIA (const jive_resource_class * const[]) 

const jive_register_class jive_testarch_regcls [] = {
	[cls_r0] = {
		.base = {
			.name = "r0",
			.limit = 1, .names = allnames + 0,
			.parent = &jive_testarch_regcls[cls_evenreg].base, .depth = 4, .is_abstract = false,
			.priority = jive_resource_class_priority_reg_low,
			.demotions = (const jive_resource_class_demotion []) {
				{STACK32, VIA {CLS(gpr), STACK32, NULL}},
				{NULL, NULL}
			},
			.type = &bits32.base.base
		},
		.regs = jive_testarch_regs + 0
	},
	[cls_r1] = {
		.base = {
			.name = "r1",
			.limit = 1, .names = allnames + 2,
			.parent = &jive_testarch_regcls[cls_oddreg].base, .depth = 4, .is_abstract = false,
			.priority = jive_resource_class_priority_reg_low,
			.demotions = (const jive_resource_class_demotion []) {
				{STACK32, VIA {CLS(gpr), STACK32, NULL}},
				{NULL, NULL}
			},
			.type = &bits32.base.base
		},
		.regs = jive_testarch_regs + 2
	},
	[cls_r2] = {
		.base = {
			.name = "r2",
			.limit = 1, .names = allnames + 1,
			.parent = &jive_testarch_regcls[cls_evenreg].base, .depth = 4, .is_abstract = false,
			.priority = jive_resource_class_priority_reg_low,
			.demotions = (const jive_resource_class_demotion []) {
				{STACK32, VIA {CLS(gpr), STACK32, NULL}},
				{NULL, NULL}
			},
			.type = &bits32.base.base
		},
		.regs = jive_testarch_regs + 1
	},
	[cls_r3] = {
		.base = {
			.name = "r3",
			.limit = 1, .names = allnames + 3,
			.parent = &jive_testarch_regcls[cls_oddreg].base, .depth = 4, .is_abstract = false,
			.priority = jive_resource_class_priority_reg_low,
			.demotions = (const jive_resource_class_demotion []) {
				{STACK32, VIA {CLS(gpr), STACK32, NULL}},
				{NULL, NULL}
			},
			.type = &bits32.base.base
		},
		.regs = jive_testarch_regs + 3
	},
	[cls_evenreg] = {
		.base = {
			.name = "even",
			.limit = 2, .names = allnames + 0,
			.parent = &jive_testarch_regcls[cls_gpr].base, .depth = 3, .is_abstract = false,
			.priority = jive_resource_class_priority_reg_low,
			.demotions = (const jive_resource_class_demotion []) {
				{STACK32, VIA {CLS(gpr), STACK32, NULL}},
				{NULL, NULL}
			},
			.type = &bits32.base.base
		},
		.regs = jive_testarch_regs + 0
	},
	[cls_oddreg] = {
		.base = {
			.name = "odd",
			.limit = 2, .names = allnames + 2,
			.parent = &jive_testarch_regcls[cls_gpr].base, .depth = 3, .is_abstract = false,
			.priority = jive_resource_class_priority_reg_low,
			.demotions = (const jive_resource_class_demotion []) {
				{STACK32, VIA {CLS(gpr), STACK32, NULL}},
				{NULL, NULL}
			},
			.type = &bits32.base.base
		},
		.regs = jive_testarch_regs + 2
	},
	[cls_gpr] = {
		.base = {
			.name = "gpr",
			.limit = 4, .names = allnames + 0,
			.parent = &jive_root_register_class, .depth = 2, .is_abstract = false,
			.priority = jive_resource_class_priority_reg_low,
			.demotions = (const jive_resource_class_demotion []) {
				{STACK32, VIA {CLS(gpr), STACK32, NULL}},
				{NULL, NULL}
			},
			.type = &bits32.base.base
		},
		.regs = jive_testarch_regs
	},
	[cls_cc] = {
		.base = {
			.name = "cc",
			.limit = 1, .names = allnames + 4,
			.parent = &jive_root_register_class, .depth = 2, .is_abstract = false,
			.priority = jive_resource_class_priority_reg_high,
			.demotions = (const jive_resource_class_demotion []) {
				{CLS(gpr), VIA {CLS(cc), CLS(gpr), NULL}},
				{STACK32, VIA {CLS(cc), CLS(gpr), STACK32, NULL}},
				{NULL, NULL}
			},
			.type = &bits16.base.base
		},
		.regs = jive_testarch_regs + 4
	},
};

static const jive_register_class * gpr_params[] = {
	&jive_testarch_regcls[cls_gpr],
	&jive_testarch_regcls[cls_gpr]
};

static const jive_register_class * special_params[] = {
	&jive_testarch_regcls[cls_r0],
	&jive_testarch_regcls[cls_r1],
	&jive_testarch_regcls[cls_r2],
	&jive_testarch_regcls[cls_r3]
};

const jive_instruction_class jive_testarch_instructions[] = {
	[nop_index] = {
		.name = "nop",
		.mnemonic = "nop",
		.encode = 0,
		.write_asm = 0,
		.inregs = 0, .outregs = 0, .flags = jive_instruction_flags_none, .ninputs = 0, .noutputs = 0, .nimmediates = 0,
		.code = 0
	},
	
	[add_index] = {
		.name = "add",
		.mnemonic = "add",
		.encode = 0,
		.write_asm = 0,
		.inregs = gpr_params, .outregs = gpr_params, .flags = jive_instruction_write_input | jive_instruction_commutative,
		.ninputs = 2, .noutputs = 1, .nimmediates = 0,
		.code = 0
	},
	
	[load_disp_index] = {
		.name = "load_disp",
		.mnemonic = "load_disp",
		.encode = 0,
		.write_asm = 0,
		.inregs = gpr_params, .outregs = gpr_params, .flags = jive_instruction_flags_none,
		.ninputs = 1, .noutputs = 1, .nimmediates = 1,
		.code = 0
	},
	
	[store_disp_index] = {
		.name = "store_disp",
		.mnemonic = "load_disp",
		.encode = 0,
		.write_asm = 0,
		.inregs = gpr_params, .outregs = 0, .flags = jive_instruction_flags_none,
		.ninputs = 2, .noutputs = 0, .nimmediates = 1,
		.code = 0
	},
	
	[spill_gpr_index] = {
		.name = "spill_gpr",
		.mnemonic = "spill_gpr",
		.encode = 0,
		.write_asm = 0,
		.inregs = gpr_params, .outregs = 0, .flags = jive_instruction_flags_none,
		.ninputs = 1, .noutputs = 0, .nimmediates = 0,
		.code = 0
	},
	
	[restore_gpr_index] = {
		.name = "restore_gpr",
		.mnemonic = "restore_gpr",
		.encode = 0,
		.write_asm = 0,
		.inregs = 0, .outregs = gpr_params, .flags = jive_instruction_flags_none,
		.ninputs = 0, .noutputs = 1, .nimmediates = 0,
		.code = 0
	},
	
	[move_gpr_index] = {
		.name = "move_gpr",
		.mnemonic = "move_gpr",
		.encode = 0,
		.write_asm = 0,
		.inregs = gpr_params, .outregs = gpr_params, .flags = jive_instruction_flags_none,
		.ninputs = 1, .noutputs = 1, .nimmediates = 0,
		.code = 0
	},
	
	[setr0_index] = {
		.name = "setr0",
		.mnemonic = "setr0",
		.encode = 0,
		.write_asm = 0,
		.inregs = gpr_params, .outregs = &special_params[0], .flags = jive_instruction_flags_none,
		.ninputs = 1, .noutputs = 1, .nimmediates = 0,
		.code = 0
	},
	[setr1_index] = {
		.name = "setr1",
		.mnemonic = "setr1",
		.encode = 0,
		.write_asm = 0,
		.inregs = gpr_params, .outregs = &special_params[1], .flags = jive_instruction_flags_none,
		.ninputs = 1, .noutputs = 1, .nimmediates = 0,
		.code = 0
	},
	[setr2_index] = {
		.name = "setr2",
		.mnemonic = "setr2",
		.encode = 0,
		.write_asm = 0,
		.inregs = gpr_params, .outregs = &special_params[2], .flags = jive_instruction_flags_none,
		.ninputs = 1, .noutputs = 1, .nimmediates = 0,
		.code = 0
	},
	[setr3_index] = {
		.name = "setr3",
		.mnemonic = "setr3",
		.encode = 0,
		.write_asm = 0,
		.inregs = gpr_params, .outregs = &special_params[3], .flags = jive_instruction_flags_none,
		.ninputs = 1, .noutputs = 1, .nimmediates = 0,
		.code = 0
	},
	
	[add_gpr_index] = {
		.name = "add_gpr",
		.mnemonic = "add_gpr",
		.encode = 0,
		.write_asm = 0,
		.inregs = gpr_params, .outregs = gpr_params, .flags = jive_instruction_write_input | jive_instruction_commutative,
		.ninputs = 2, .noutputs = 1, .nimmediates = 0,
		.code = 0
	},
	[sub_gpr_index] = {
		.name = "sub_gpr",
		.mnemonic = "sub_gpr",
		.encode = 0,
		.write_asm = 0,
		.inregs = gpr_params, .outregs = gpr_params, .flags = jive_instruction_write_input,
		.ninputs = 2, .noutputs = 1, .nimmediates = 0,
		.code = 0
	}
};

static jive_xfer_block
create_xfer(jive_region * region, jive_output * origin,
	const jive_resource_class * in_class, const jive_resource_class * out_class)
{
	jive_xfer_block xfer;
	
	const jive_resource_class * in_relaxed = jive_resource_class_relax(in_class);
	const jive_resource_class * out_relaxed = jive_resource_class_relax(out_class);
	
	if (in_relaxed == CLS(gpr) && out_relaxed == CLS(gpr)) {
		xfer.node = jive_instruction_node_create(
			region,
			&jive_testarch_instructions[move_gpr_index],
			(jive_output *[]){origin}, NULL);
		xfer.input = xfer.node->inputs[0];
		xfer.output = xfer.node->outputs[0];
	} else if (in_relaxed == CLS(gpr)) {
		xfer.node = jive_instruction_node_create(
			region,
			&jive_testarch_instructions[spill_gpr_index],
			(jive_output *[]){origin}, NULL);
		xfer.input = xfer.node->inputs[0];
		xfer.output = jive_node_add_output(xfer.node, jive_resource_class_get_type(out_class));
	} else if (out_relaxed == CLS(gpr)) {
		xfer.node = jive_instruction_node_create(
			region,
			&jive_testarch_instructions[restore_gpr_index],
			NULL, NULL);
		xfer.input = jive_node_add_input(xfer.node, jive_resource_class_get_type(in_class), origin);
		xfer.output = xfer.node->outputs[0];
	} else {
		JIVE_DEBUG_ASSERT(false);
	}
	
	return xfer;
}

const jive_transfer_instructions_factory jive_testarch_xfer_factory = {
	create_xfer
};

static void
jive_testarch_subroutine_fini_(jive_subroutine * self_)
{
	jive_testarch_subroutine * self = (jive_testarch_subroutine *) self_;
	jive_context_free(self->base.context, self->base.parameters);
	jive_context_free(self->base.context, self->base.returns);
}

static jive_output *
jive_testarch_subroutine_value_parameter_(jive_subroutine * self_, size_t index)
{
	jive_testarch_subroutine * self = (jive_testarch_subroutine *) self_;
	jive_gate * gate = self->base.parameters[index];
	jive_output * output = jive_node_gate_output(&self->base.enter->base, gate);
	if (index >= 2) {
		const jive_type * in_type = jive_gate_get_type(gate);
		const jive_type * out_type = jive_resource_class_get_type(jive_testarch_cls_gpr);
		jive_node * node = jive_aux_split_node_create(self->base.enter->base.region,
			in_type, output, gate->required_rescls,
			out_type, jive_testarch_cls_gpr);
		output = node->outputs[0];
	}
	return output;
}

static jive_input *
jive_testarch_subroutine_value_return_(jive_subroutine * self_, size_t index, jive_output * value)
{
	jive_testarch_subroutine * self = (jive_testarch_subroutine *) self_;
	jive_gate * gate = self->base.returns[index];
	return jive_node_gate_input(&self->base.leave->base, gate, value);
}

static jive_testarch_subroutine *
jive_testarch_subroutine_alloc(jive_region * region, size_t nparameters, size_t nreturns);

static jive_subroutine *
jive_testarch_subroutine_copy_(const jive_subroutine * self_,
	jive_node * new_enter_node, jive_node * new_leave_node)
{
	jive_graph * graph = new_enter_node->region->graph;
	jive_testarch_subroutine * self = (jive_testarch_subroutine *) self_;
	
	jive_testarch_subroutine * other = jive_testarch_subroutine_alloc(new_enter_node->region, self->base.nparameters, self->base.nreturns);
	other->base.enter = (jive_subroutine_enter_node *) new_enter_node;
	other->base.leave = (jive_subroutine_leave_node *) new_leave_node;
	
	size_t n;
	
	for (n = 0; n < self->base.nparameters; n++) {
		jive_gate * old_gate = self->base.parameters[n];
		jive_gate * new_gate = NULL;
		
		if (!new_gate)
			new_gate = jive_subroutine_match_gate(old_gate, &self->base.enter->base, new_enter_node);
		if (!new_gate)
			new_gate = jive_subroutine_match_gate(old_gate, &self->base.leave->base, new_leave_node);
		if (!new_gate)
			new_gate = jive_resource_class_create_gate(old_gate->required_rescls, graph, old_gate->name);
		
		other->base.parameters[n] = new_gate;
	}
	
	for (n = 0; n < self->base.nreturns; n++) {
		jive_gate * old_gate = self->base.returns[n];
		jive_gate * new_gate = NULL;
		
		if (!new_gate)
			new_gate = jive_subroutine_match_gate(old_gate, &self->base.enter->base, new_enter_node);
		if (!new_gate)
			new_gate = jive_subroutine_match_gate(old_gate, &self->base.leave->base, new_leave_node);
		if (!new_gate)
			new_gate = jive_resource_class_create_gate(old_gate->required_rescls, graph, old_gate->name);
		
		other->base.returns[n] = new_gate;
	}
	
	jive_subroutine_match_passthrough(&self->base, &self->stackptr, &other->base, &other->stackptr);
	
	return &other->base;
}

static const jive_subroutine_class JIVE_TESTARCH_SUBROUTINE = {
	.fini = jive_testarch_subroutine_fini_,
	.value_parameter = jive_testarch_subroutine_value_parameter_,
	.value_return = jive_testarch_subroutine_value_return_,
	.copy = jive_testarch_subroutine_copy_
};

static jive_testarch_subroutine *
jive_testarch_subroutine_alloc(jive_region * region, size_t nparameters, size_t nreturns)
{
	jive_graph * graph = region->graph;
	jive_context * context = graph->context;
	
	jive_testarch_subroutine * self;
	self = jive_context_malloc(context, sizeof(*self));
	jive_subroutine_init(&self->base, &JIVE_TESTARCH_SUBROUTINE, context);
	
	size_t n;
	
	self->base.nparameters = nparameters;
	self->base.parameters = jive_context_malloc(context, sizeof(self->base.parameters[0]) * nparameters);
	
	for (n = 0; n < nparameters; n++)
		self->base.parameters[n] = NULL;
	
	self->base.nreturns = nreturns;
	self->base.returns = jive_context_malloc(context, sizeof(self->base.returns[0]) * nreturns);
	
	for (n = 0; n < nreturns; n++)
		self->base.returns[n] = NULL;
	
	jive_subroutine_init(&self->base, &JIVE_TESTARCH_SUBROUTINE, context);
	
	return self;
}

jive_subroutine *
jive_testarch_subroutine_create(jive_region * region,
	size_t nparameters, const jive_argument_type parameters[],
	size_t nreturns, const jive_argument_type returns[])
{
	jive_graph * graph = region->graph;
	jive_testarch_subroutine * self = jive_testarch_subroutine_alloc(region, nparameters, nreturns);
	
	size_t n;
	
	for (n = 0; n < nparameters; n++) {
		char argname[80];
		snprintf(argname, sizeof(argname), "arg%zd", n + 1);
		const jive_resource_class * cls;
		switch (n) {
			case 0: cls = jive_testarch_cls_r1; break;
			case 1: cls = jive_testarch_cls_r2; break;
			default: cls = jive_fixed_stackslot_class_get(32, (n - 1) * 4);
		}
		self->base.parameters[n] = jive_resource_class_create_gate(cls, graph, argname);
	}
	
	for (n = 0; n < nreturns; n++) {
		char argname[80];
		snprintf(argname, sizeof(argname), "ret%zd", n + 1);
		const jive_resource_class * cls;
		switch (n) {
			case 0: cls = jive_testarch_cls_r1; break;
			default: cls = jive_fixed_stackslot_class_get(32, n * 4);
		}
		self->base.returns[n] = jive_resource_class_create_gate(cls, graph, argname);
	}
	
	jive_subroutine_create_region_and_nodes(&self->base, region);
	
	self->stackptr = jive_subroutine_create_passthrough(&self->base, jive_testarch_cls_r0, "stackptr");
	self->stackptr.gate->may_spill = false;
	
	return &self->base;
}
