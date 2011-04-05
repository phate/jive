#include <jive/backend/i386/registerset.h>

#include <jive/arch/registers.h>

const jive_register_name jive_i386_regs [] = {
	[jive_i386_cc] = {.base = {.name = "cc", .resource_class = &jive_i386_regcls[jive_i386_flags].base}, .code = 0},
	[jive_i386_eax] = {.base = {.name = "eax", .resource_class = &jive_i386_regcls[jive_i386_gpr_eax].base}, .code = 0},
	[jive_i386_ecx] = {.base = {.name = "ecx", .resource_class = &jive_i386_regcls[jive_i386_gpr_ecx].base}, .code = 1},
	[jive_i386_ebx] = {.base = {.name = "ebx", .resource_class = &jive_i386_regcls[jive_i386_gpr_ebx].base}, .code = 2},
	[jive_i386_edx] = {.base = {.name = "edx", .resource_class = &jive_i386_regcls[jive_i386_gpr_edx].base}, .code = 3},
	[jive_i386_esi] = {.base = {.name = "esi", .resource_class = &jive_i386_regcls[jive_i386_gpr_esi].base}, .code = 6},
	[jive_i386_edi] = {.base = {.name = "edi", .resource_class = &jive_i386_regcls[jive_i386_gpr_edi].base}, .code = 7},
	[jive_i386_ebp] = {.base = {.name = "ebp", .resource_class = &jive_i386_regcls[jive_i386_gpr_ebp].base}, .code = 5},
	[jive_i386_esp] = {.base = {.name = "esp", .resource_class = &jive_i386_regcls[jive_i386_gpr_esp].base}, .code = 4},
};

static const jive_resource_name * jive_i386_names [] = {
	[jive_i386_cc] = &jive_i386_regs[jive_i386_cc].base,
	[jive_i386_eax] = &jive_i386_regs[jive_i386_eax].base,
	[jive_i386_ecx] = &jive_i386_regs[jive_i386_ecx].base,
	[jive_i386_ebx] = &jive_i386_regs[jive_i386_ebx].base,
	[jive_i386_edx] = &jive_i386_regs[jive_i386_edx].base,
	[jive_i386_esi] = &jive_i386_regs[jive_i386_esi].base,
	[jive_i386_edi] = &jive_i386_regs[jive_i386_edi].base,
	[jive_i386_ebp] = &jive_i386_regs[jive_i386_ebp].base,
	[jive_i386_esp] = &jive_i386_regs[jive_i386_esp].base
};

const jive_register_class jive_i386_regcls [] = {
	[jive_i386_flags] = {
		.base = {
			.name = "flags",
			.limit = 1, .names = &jive_i386_names[jive_i386_cc],
			.parent = &jive_root_resource_class, .depth = 1
		},
		.regs = &jive_i386_regs[jive_i386_cc],
		.nbits = 16
	},
	[jive_i386_gpr] = {
		.base = {
			.name = "gpr",
			.limit = 8, .names = &jive_i386_names[jive_i386_eax],
			.parent = &jive_root_resource_class, .depth = 1
		},
		.regs = &jive_i386_regs[jive_i386_eax],
		.nbits = 32, .int_arithmetic_width = 32, .loadstore_width = 8|16|32
	},
	[jive_i386_gpr_byte] = {
		.base = {
			.name = "gpr_byte_addressible",
			.limit = 4, .names = &jive_i386_names[jive_i386_eax],
			.parent = &jive_i386_regcls[jive_i386_gpr].base, .depth = 2
		},
		.regs = &jive_i386_regs[jive_i386_eax],
		.nbits = 32, .int_arithmetic_width = 32, .loadstore_width = 8|16|32
	},
	[jive_i386_gpr_eax] = {
		.base = {
			.name = "gpr_eax",
			.limit = 1, .names = &jive_i386_names[jive_i386_eax],
			.parent = &jive_i386_regcls[jive_i386_gpr_byte].base, .depth = 3
		},
		.regs = &jive_i386_regs[jive_i386_eax],
		.nbits = 32, .int_arithmetic_width = 32, .loadstore_width = 8|16|32
	},
	[jive_i386_gpr_ecx] = {
		.base = {
			.name = "gpr_ecx",
			.limit = 1, .names = &jive_i386_names[jive_i386_ecx],
			.parent = &jive_i386_regcls[jive_i386_gpr_byte].base, .depth = 3
		},
		.regs = &jive_i386_regs[jive_i386_ecx],
		.nbits = 32, .int_arithmetic_width = 32, .loadstore_width = 8|16|32
	},
	[jive_i386_gpr_ebx] = {
		.base = {
			.name = "gpr_ebx",
			.limit = 1, .names = &jive_i386_names[jive_i386_ebx],
			.parent = &jive_i386_regcls[jive_i386_gpr_byte].base, .depth = 3
		},
		.regs = &jive_i386_regs[jive_i386_ebx],
		.nbits = 32, .int_arithmetic_width = 32, .loadstore_width = 8|16|32
	},
	[jive_i386_gpr_edx] = {
		.base = {
			.name = "gpr_edx",
			.limit = 1, .names = &jive_i386_names[jive_i386_edx],
			.parent = &jive_i386_regcls[jive_i386_gpr_byte].base, .depth = 3
		},
		.regs = &jive_i386_regs[jive_i386_edx],
		.nbits = 32, .int_arithmetic_width = 32, .loadstore_width = 8|16|32
	},
	[jive_i386_gpr_esi] = {
		.base = {
			.name = "gpr_esi",
			.limit = 1, .names = &jive_i386_names[jive_i386_esi],
			.parent = &jive_i386_regcls[jive_i386_gpr].base, .depth = 2
		},
		.regs = &jive_i386_regs[jive_i386_esi],
		.nbits = 32, .int_arithmetic_width = 32, .loadstore_width = 8|16|32
	},
	[jive_i386_gpr_edi] = {
		.base = {
			.name = "gpr_edi",
			.limit = 1, .names = &jive_i386_names[jive_i386_edi],
			.parent = &jive_i386_regcls[jive_i386_gpr].base, .depth = 2
		},
		.regs = &jive_i386_regs[jive_i386_edi],
		.nbits = 32, .int_arithmetic_width = 32, .loadstore_width = 8|16|32
	},
	[jive_i386_gpr_esp] = {
		.base = {
			.name = "gpr_esp",
			.limit = 1, .names = &jive_i386_names[jive_i386_esp],
			.parent = &jive_i386_regcls[jive_i386_gpr].base, .depth = 2
		},
		.regs = &jive_i386_regs[jive_i386_esp],
		.nbits = 32, .int_arithmetic_width = 32, .loadstore_width = 8|16|32
	},
	[jive_i386_gpr_ebp] = {
		.base = {
			.name = "gpr_ebp",
			.limit = 1, .names = &jive_i386_names[jive_i386_ebp],
			.parent = &jive_i386_regcls[jive_i386_gpr].base, .depth = 2
		},
		.regs = &jive_i386_regs[jive_i386_ebp],
		.nbits = 32, .int_arithmetic_width = 32, .loadstore_width = 8|16|32
	}
};
