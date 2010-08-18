#include "testarch.h"

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
};
