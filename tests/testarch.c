#include "testarch.h"

#include <jive/arch/registers.h>
#include <jive/bitstring/type.h>

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

const jive_register_class jive_testarch_regcls [] = {
	[cls_r0] = {
		.base = {
			.name = "r0",
			.limit = 1, .names = allnames + 0,
			.parent = &jive_testarch_regcls[cls_evenreg].base, .depth = 3,
			.priority = jive_resource_class_priority_reg_low,
			.demotions = (const jive_resource_class_demotion []) {{NULL, NULL}},
			.type = &bits32.base.base
		},
		.regs = jive_testarch_regs + 0
	},
	[cls_r1] = {
		.base = {
			.name = "r1",
			.limit = 1, .names = allnames + 2,
			.parent = &jive_testarch_regcls[cls_oddreg].base, .depth = 3,
			.priority = jive_resource_class_priority_reg_low,
			.demotions = (const jive_resource_class_demotion []) {{NULL, NULL}},
			.type = &bits32.base.base
		},
		.regs = jive_testarch_regs + 2
	},
	[cls_r2] = {
		.base = {
			.name = "r2",
			.limit = 1, .names = allnames + 1,
			.parent = &jive_testarch_regcls[cls_evenreg].base, .depth = 3,
			.priority = jive_resource_class_priority_reg_low,
			.demotions = (const jive_resource_class_demotion []) {{NULL, NULL}},
			.type = &bits32.base.base
		},
		.regs = jive_testarch_regs + 1
	},
	[cls_r3] = {
		.base = {
			.name = "r3",
			.limit = 1, .names = allnames + 3,
			.parent = &jive_testarch_regcls[cls_oddreg].base, .depth = 3,
			.priority = jive_resource_class_priority_reg_low,
			.demotions = (const jive_resource_class_demotion []) {{NULL, NULL}},
			.type = &bits32.base.base
		},
		.regs = jive_testarch_regs + 3
	},
	[cls_evenreg] = {
		.base = {
			.name = "even",
			.limit = 2, .names = allnames + 0,
			.parent = &jive_testarch_regcls[cls_gpr].base, .depth = 2,
			.priority = jive_resource_class_priority_reg_low,
			.demotions = (const jive_resource_class_demotion []) {{NULL, NULL}},
			.type = &bits32.base.base
		},
		.regs = jive_testarch_regs + 0
	},
	[cls_oddreg] = {
		.base = {
			.name = "odd",
			.limit = 2, .names = allnames + 2,
			.parent = &jive_testarch_regcls[cls_gpr].base, .depth = 2,
			.priority = jive_resource_class_priority_reg_low,
			.demotions = (const jive_resource_class_demotion []) {{NULL, NULL}},
			.type = &bits32.base.base
		},
		.regs = jive_testarch_regs + 2
	},
	[cls_gpr] = {
		.base = {
			.name = "gpr",
			.limit = 4, .names = allnames + 0,
			.parent = &jive_root_resource_class, .depth = 1,
			.priority = jive_resource_class_priority_reg_low,
			.demotions = (const jive_resource_class_demotion []) {{NULL, NULL}},
			.type = &bits32.base.base
		},
		.regs = jive_testarch_regs
	},
	[cls_cc] = {
		.base = {
			.name = "cc",
			.limit = 1, .names = allnames + 4,
			.parent = &jive_root_resource_class, .depth = 1,
			.priority = jive_resource_class_priority_reg_high,
			.demotions = (const jive_resource_class_demotion []) {{NULL, NULL}},
			.type = &bits16.base.base
		},
		.regs = jive_testarch_regs + 4
	},
};

static const jive_register_class * gpr_params[] = {
	&jive_testarch_regcls[cls_gpr],
	&jive_testarch_regcls[cls_gpr]
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
	}
};
