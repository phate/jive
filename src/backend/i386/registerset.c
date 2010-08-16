#include <jive/backend/i386/registerset.h>

#include <jive/arch/registers.h>

const jive_cpureg jive_i386_regs [] = {
	[jive_i386_cc] = {.name = "cc", .regcls = &jive_i386_regcls[jive_i386_flags], .code = 0, .index = jive_i386_cc},
	[jive_i386_eax] = {.name = "eax", .regcls = &jive_i386_regcls[jive_i386_gpr_eax], .code = 0, .index = jive_i386_eax},
	[jive_i386_ecx] = {.name = "ecx", .regcls = &jive_i386_regcls[jive_i386_gpr_ecx], .code = 1, .index = jive_i386_ecx},
	[jive_i386_ebx] = {.name = "ebx", .regcls = &jive_i386_regcls[jive_i386_gpr_ebx], .code = 2, .index = jive_i386_ebx},
	[jive_i386_edx] = {.name = "edx", .regcls = &jive_i386_regcls[jive_i386_gpr_edx], .code = 3, .index = jive_i386_edx},
	[jive_i386_esi] = {.name = "esi", .regcls = &jive_i386_regcls[jive_i386_gpr_esi], .code = 6, .index = jive_i386_esi},
	[jive_i386_edi] = {.name = "edi", .regcls = &jive_i386_regcls[jive_i386_gpr_edi], .code = 7, .index = jive_i386_edi},
	[jive_i386_ebp] = {.name = "ebp", .regcls = &jive_i386_regcls[jive_i386_gpr_ebp], .code = 5, .index = jive_i386_ebp},
	[jive_i386_esp] = {.name = "esp", .regcls = &jive_i386_regcls[jive_i386_gpr_esp], .code = 4, .index = jive_i386_esp},
};

const jive_regcls jive_i386_regcls [] = {
	[jive_i386_flags] = {
		.name = "flags", .nbits = 16,
		.regs = &jive_i386_regs[jive_i386_cc], .nregs = 1,
		.index = jive_i386_flags, .parent = 0, .depth = 0
	},
	[jive_i386_gpr] = {
		.name = "gpr", .nbits = 32, .int_arithmetic_width = 32, .loadstore_width = 8|16|32,
		.regs = &jive_i386_regs[jive_i386_eax], .nregs = 8,
		.index = jive_i386_gpr, .parent = 0, .depth = 0
	},
	[jive_i386_gpr_byte] = {
		.name = "gpr_byte_addressible", .nbits = 32, .int_arithmetic_width = 32, .loadstore_width = 8|16|32,
		.regs = &jive_i386_regs[jive_i386_eax], .nregs = 4,
		.index = jive_i386_gpr_byte, .parent = &jive_i386_regcls[jive_i386_gpr], .depth = 1
	},
	[jive_i386_gpr_eax] = {
		.name = "gpr_eax", .nbits = 32, .int_arithmetic_width = 32, .loadstore_width = 8|16|32,
		.regs = &jive_i386_regs[jive_i386_eax], .nregs = 1,
		.index = jive_i386_gpr_eax, .parent = &jive_i386_regcls[jive_i386_gpr_byte], .depth = 2
	},
	[jive_i386_gpr_ecx] = {
		.name = "gpr_ecx", .nbits = 32, .int_arithmetic_width = 32, .loadstore_width = 8|16|32,
		.regs = &jive_i386_regs[jive_i386_ecx], .nregs = 1,
		.index = jive_i386_gpr_ecx, .parent = &jive_i386_regcls[jive_i386_gpr_byte], .depth = 2
	},
	[jive_i386_gpr_ebx] = {
		.name = "gpr_ebx", .nbits = 32, .int_arithmetic_width = 32, .loadstore_width = 8|16|32,
		.regs = &jive_i386_regs[jive_i386_ebx], .nregs = 1,
		.index = jive_i386_gpr_ebx, .parent = &jive_i386_regcls[jive_i386_gpr_byte], .depth = 2
	},
	[jive_i386_gpr_edx] = {
		.name = "gpr_edx", .nbits = 32, .int_arithmetic_width = 32, .loadstore_width = 8|16|32,
		.regs = &jive_i386_regs[jive_i386_edx], .nregs = 1,
		.index = jive_i386_gpr_edx, .parent = &jive_i386_regcls[jive_i386_gpr_byte], .depth = 2
	},
	[jive_i386_gpr_esi] = {
		.name = "gpr_esi", .nbits = 32, .int_arithmetic_width = 32, .loadstore_width = 8|16|32,
		.regs = &jive_i386_regs[jive_i386_esi], .nregs = 1,
		.index = jive_i386_gpr_esi, .parent = &jive_i386_regcls[jive_i386_gpr], .depth = 1
	},
	[jive_i386_gpr_edi] = {
		.name = "gpr_edi", .nbits = 32, .int_arithmetic_width = 32, .loadstore_width = 8|16|32,
		.regs = &jive_i386_regs[jive_i386_edi], .nregs = 1,
		.index = jive_i386_gpr_edi, .parent = &jive_i386_regcls[jive_i386_gpr], .depth = 1
	},
	[jive_i386_gpr_esp] = {
		.name = "gpr_esp", .nbits = 32, .int_arithmetic_width = 32, .loadstore_width = 8|16|32,
		.regs = &jive_i386_regs[jive_i386_esp], .nregs = 1,
		.index = jive_i386_gpr_esp, .parent = &jive_i386_regcls[jive_i386_gpr], .depth = 1
	},
	[jive_i386_gpr_ebp] = {
		.name = "gpr_ebp", .nbits = 32, .int_arithmetic_width = 32, .loadstore_width = 8|16|32,
		.regs = &jive_i386_regs[jive_i386_ebp], .nregs = 1,
		.index = jive_i386_gpr_ebp, .parent = &jive_i386_regcls[jive_i386_gpr], .depth = 1
	}
};
