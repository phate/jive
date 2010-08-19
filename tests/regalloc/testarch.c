#include "testarch.h"

#include <jive/arch/transfer-instructions.h>
#include <jive/arch/stackframe.h>
#include <jive/vsdg.h>

const jive_cpureg jive_testarch_regs [] = {
	[reg_r1] = {.name = "r1", .regcls = &jive_testarch_regcls[cls_r1], .index = reg_r1},
	[reg_r2] = {.name = "r2", .regcls = &jive_testarch_regcls[cls_r2], .index = reg_r2},
	[reg_r3] = {.name = "r3", .regcls = &jive_testarch_regcls[cls_r3], .index = reg_r3},
	[reg_r4] = {.name = "r4", .regcls = &jive_testarch_regcls[cls_r4], .index = reg_r4}
};

const jive_regcls jive_testarch_regcls [] = {
	[cls_regs] = {
		.name = "gpr", .nbits = 32,
		.regs = &jive_testarch_regs[reg_r1], .nregs = 4,
		.index = cls_regs, .parent = 0, .depth = 0
	},
	[cls_r1] = {
		.name = "r1", .nbits = 32,
		.regs = &jive_testarch_regs[reg_r1], .nregs = 1,
		.index = cls_regs, .parent = &jive_testarch_regcls[cls_regs], .depth = 1
	},
	[cls_r2] = {
		.name = "r2", .nbits = 32,
		.regs = &jive_testarch_regs[reg_r2], .nregs = 1,
		.index = cls_regs, .parent = &jive_testarch_regcls[cls_regs], .depth = 1
	},
	[cls_r3] = {
		.name = "r3", .nbits = 32,
		.regs = &jive_testarch_regs[reg_r3], .nregs = 1,
		.index = cls_regs, .parent = &jive_testarch_regcls[cls_regs], .depth = 1
	},
	[cls_r4] = {
		.name = "r4", .nbits = 32,
		.regs = &jive_testarch_regs[reg_r4], .nregs = 1,
		.index = cls_regs, .parent = &jive_testarch_regcls[cls_regs], .depth = 1
	},
};

static const jive_regcls * gpr_params[] = {
	&jive_testarch_regcls[cls_regs],
	&jive_testarch_regcls[cls_regs]
};

static const jive_regcls * special_params[] = {
	&jive_testarch_regcls[cls_r1],
	&jive_testarch_regcls[cls_r2],
	&jive_testarch_regcls[cls_r3],
	&jive_testarch_regcls[cls_r4]
};

const jive_instruction_class jive_testarch_instructions[] = {
	[instr_nop] = {
		.name = "nop", .encode = 0, .mnemonic = 0,
		.inregs = 0, .outregs = 0, .flags = jive_instruction_flags_none,
		.ninputs = 0, .noutputs = 0, .nimmediates = 0
	},
	[instr_setr1] = {
		.name = "setr1", .encode = 0, .mnemonic = 0,
		.inregs = gpr_params, .outregs = &special_params[0], .flags = jive_instruction_flags_none,
		.ninputs = 1, .noutputs = 1, .nimmediates = 0
	},
	[instr_setr2] = {
		.name = "setr2", .encode = 0, .mnemonic = 0,
		.inregs = gpr_params, .outregs = &special_params[1], .flags = jive_instruction_flags_none,
		.ninputs = 1, .noutputs = 1, .nimmediates = 0
	},
	[instr_setr3] = {
		.name = "setr3", .encode = 0, .mnemonic = 0,
		.inregs = gpr_params, .outregs = &special_params[2], .flags = jive_instruction_flags_none,
		.ninputs = 1, .noutputs = 1, .nimmediates = 0
	},
	[instr_setr4] = {
		.name = "setr4", .encode = 0, .mnemonic = 0,
		.inregs = gpr_params, .outregs = &special_params[3], .flags = jive_instruction_flags_none,
		.ninputs = 1, .noutputs = 1, .nimmediates = 0
	},
	
	[instr_getr1] = {
		.name = "getr1", .encode = 0, .mnemonic = 0,
		.inregs = &special_params[0], .outregs = gpr_params, .flags = jive_instruction_flags_none,
		.ninputs = 1, .noutputs = 1, .nimmediates = 0
	},
	[instr_getr2] = {
		.name = "getr2", .encode = 0, .mnemonic = 0,
		.inregs = &special_params[1], .outregs = gpr_params, .flags = jive_instruction_flags_none,
		.ninputs = 1, .noutputs = 1, .nimmediates = 0
	},
	[instr_getr3] = {
		.name = "getr3", .encode = 0, .mnemonic = 0,
		.inregs = &special_params[2], .outregs = gpr_params, .flags = jive_instruction_flags_none,
		.ninputs = 1, .noutputs = 1, .nimmediates = 0
	},
	[instr_getr4] = {
		.name = "getr4", .encode = 0, .mnemonic = 0,
		.inregs = &special_params[3], .outregs = gpr_params, .flags = jive_instruction_flags_none,
		.ninputs = 1, .noutputs = 1, .nimmediates = 0
	},
	
	[instr_add] = {
		.name = "add", .encode = 0, .mnemonic = 0,
		.inregs = gpr_params, .outregs = gpr_params, .flags = jive_instruction_write_input | jive_instruction_commutative,
		.ninputs = 2, .noutputs = 1, .nimmediates = 0
	},
	[instr_sub] = {
		.name = "sub", .encode = 0, .mnemonic = 0,
		.inregs = gpr_params, .outregs = gpr_params, .flags = jive_instruction_write_input,
		.ninputs = 2, .noutputs = 1, .nimmediates = 0
	},
	
	[instr_copy] = {
		.name = "copy", .encode = 0, .mnemonic = 0,
		.inregs = gpr_params, .outregs = gpr_params, .flags = jive_instruction_flags_none,
		.ninputs = 1, .noutputs = 1, .nimmediates = 0
	},
	
	[instr_load_disp] = {
		.name = "load_disp", .encode = 0, .mnemonic = 0,
		.inregs = gpr_params, .outregs = gpr_params, .flags = jive_instruction_flags_none,
		.ninputs = 1, .noutputs = 1, .nimmediates = 1
	},
	[instr_store_disp] = {
		.name = "store_disp", .encode = 0, .mnemonic = 0,
		.inregs = gpr_params, .outregs = gpr_params, .flags = jive_instruction_flags_none,
		.ninputs = 2, .noutputs = 0, .nimmediates = 1
	},
};


static size_t
testarch_create_copy(jive_region * region, jive_output * origin,
	jive_input ** xfer_in, jive_node * xfer_nodes[], jive_output ** xfer_out)
{
	jive_node * node = (jive_node *) jive_instruction_node_create(
		region, &jive_testarch_instructions[instr_copy],
		&origin, NULL);
	
	*xfer_in = node->inputs[0];
	*xfer_out = node->outputs[0];
	xfer_nodes[0] = node;
	return 1;
}

static size_t
testarch_create_spill(jive_region * region, jive_output * origin,
	jive_input **spill_in, jive_node * spill_nodes[], jive_node ** store_node)
{
	jive_stackframe * stack = jive_region_get_stackframe(region);
	
	long displacement = 0;
	jive_node * node = (jive_node *) jive_instruction_node_create(
		region, &jive_testarch_instructions[instr_store_disp],
		(jive_output *[]){(jive_output *) stack->stackptr, origin}, &displacement);
	
	jive_resource_assign_input(stack->stackptr->base.resource, node->inputs[0]);
	
	*spill_in = node->inputs[1];
	spill_nodes[0] = node;
	*store_node = node;
	return 1;
}

static size_t
testarch_create_restore(jive_region * region, jive_output * stackslot,
	jive_node ** load_node, jive_node * restore_nodes[], jive_output ** restore_out)
{
	jive_stackframe * stack = jive_region_get_stackframe(region);
	
	long displacement = 0;
	jive_node * node = (jive_node *) jive_instruction_node_create(
		region, &jive_testarch_instructions[instr_load_disp],
		(jive_output *[]){(jive_output *) stack->stackptr}, &displacement);
	
	jive_resource_assign_input(stack->stackptr->base.resource, node->inputs[0]);
	
	*load_node = node;
	restore_nodes[0] = node;
	*restore_out = node->outputs[0];
	return 1;
}

const jive_transfer_instructions_factory testarch_transfer_instructions_factory = {
	.create_copy = testarch_create_copy,
	.create_spill = testarch_create_spill,
	.create_restore = testarch_create_restore,
	.max_nodes = 1
};
