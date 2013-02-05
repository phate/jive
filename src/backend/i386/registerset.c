/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/backend/i386/registerset.h>

#include <jive/arch/registers.h>
#include <jive/arch/stackslot.h>
#include <jive/serialization/rescls-registry.h>
#include <jive/types/bitstring/type.h>
#include <jive/types/float/flttype.h>

const jive_register_name jive_i386_regs [] = {
	[jive_i386_cc] = {.base = {.name = "cc",
		.resource_class = &jive_i386_regcls[jive_i386_flags].base}, .code = 0},
	[jive_i386_eax] = {.base = {.name = "eax",
		.resource_class = &jive_i386_regcls[jive_i386_gpr_eax].base}, .code = 0},
	[jive_i386_ecx] = {.base = {.name = "ecx",
		.resource_class = &jive_i386_regcls[jive_i386_gpr_ecx].base}, .code = 1},
	[jive_i386_edx] = {.base = {.name = "edx",
		.resource_class = &jive_i386_regcls[jive_i386_gpr_edx].base}, .code = 2},
	[jive_i386_ebx] = {.base = {.name = "ebx",
		.resource_class = &jive_i386_regcls[jive_i386_gpr_ebx].base}, .code = 3},
	[jive_i386_esi] = {.base = {.name = "esi",
		.resource_class = &jive_i386_regcls[jive_i386_gpr_esi].base}, .code = 6},
	[jive_i386_edi] = {.base = {.name = "edi",
		.resource_class = &jive_i386_regcls[jive_i386_gpr_edi].base}, .code = 7},
	[jive_i386_ebp] = {.base = {.name = "ebp",
		.resource_class = &jive_i386_regcls[jive_i386_gpr_ebp].base}, .code = 5},
	[jive_i386_esp] = {.base = {.name = "esp",
		.resource_class = &jive_i386_regcls[jive_i386_gpr_esp].base}, .code = 4},

	[jive_i386_st0] = {.base = {.name = "st0",
		.resource_class = &jive_i386_regcls[jive_i386_fp_st0].base}, .code = 0},

	[jive_i386_xmm0] = {.base = {.name = "xmm0",
		.resource_class = &jive_i386_regcls[jive_i386_sse_xmm0].base}, .code = 0},
	[jive_i386_xmm1] = {.base = {.name = "xmm1",
		.resource_class = &jive_i386_regcls[jive_i386_sse_xmm1].base}, .code = 1},
	[jive_i386_xmm2] = {.base = {.name = "xmm2",
		.resource_class = &jive_i386_regcls[jive_i386_sse_xmm2].base}, .code = 2},
	[jive_i386_xmm3] = {.base = {.name = "xmm3",
		.resource_class = &jive_i386_regcls[jive_i386_sse_xmm3].base}, .code = 3},
	[jive_i386_xmm4] = {.base = {.name = "xmm4",
		.resource_class = &jive_i386_regcls[jive_i386_sse_xmm4].base}, .code = 4},
	[jive_i386_xmm5] = {.base = {.name = "xmm5",
		.resource_class = &jive_i386_regcls[jive_i386_sse_xmm5].base}, .code = 5},
	[jive_i386_xmm6] = {.base = {.name = "xmm6",
		.resource_class = &jive_i386_regcls[jive_i386_sse_xmm6].base}, .code = 6},
	[jive_i386_xmm7] = {.base = {.name = "xmm7",
		.resource_class = &jive_i386_regcls[jive_i386_sse_xmm7].base}, .code = 7},
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
	[jive_i386_esp] = &jive_i386_regs[jive_i386_esp].base,

	[jive_i386_st0] = &jive_i386_regs[jive_i386_st0].base,

	[jive_i386_xmm0] = &jive_i386_regs[jive_i386_xmm0].base,
	[jive_i386_xmm1] = &jive_i386_regs[jive_i386_xmm1].base,
	[jive_i386_xmm2] = &jive_i386_regs[jive_i386_xmm2].base,
	[jive_i386_xmm3] = &jive_i386_regs[jive_i386_xmm3].base,
	[jive_i386_xmm4] = &jive_i386_regs[jive_i386_xmm4].base,
	[jive_i386_xmm5] = &jive_i386_regs[jive_i386_xmm5].base,
	[jive_i386_xmm6] = &jive_i386_regs[jive_i386_xmm6].base,
	[jive_i386_xmm7] = &jive_i386_regs[jive_i386_xmm7].base,
};

static const jive_bitstring_type bits16 = {{{&JIVE_BITSTRING_TYPE}}, 16};
static const jive_bitstring_type bits32 = {{{&JIVE_BITSTRING_TYPE}}, 32};
static const jive_float_type flt = {{{&JIVE_FLOAT_TYPE}}};

#define CLS(x) &jive_i386_regcls[jive_i386_##x].base
#define STACK4 &jive_stackslot_class_4_4.base
#define VIA (const jive_resource_class * const[]) 

const jive_register_class jive_i386_regcls [] = {
	[jive_i386_flags] = {
		.base = {
			.class_ = &JIVE_REGISTER_RESOURCE,
			.name = "flags",
			.limit = 1, .names = &jive_i386_names[jive_i386_cc],
			.parent = &jive_root_register_class, .depth = 2,
			.priority = jive_resource_class_priority_reg_high,
			.demotions = (const jive_resource_class_demotion []){
				{CLS(eax), VIA {CLS(flags), CLS(eax), NULL}},
				{STACK4, VIA {CLS(flags), CLS(eax), STACK4, NULL}},
				{NULL, NULL}
			},
			.type = &bits16.base.base
		},
		.regs = &jive_i386_regs[jive_i386_cc],
		.nbits = 16
	},
	[jive_i386_gpr] = {
		.base = {
			.class_ = &JIVE_REGISTER_RESOURCE,
			.name = "gpr",
			.limit = 8, .names = &jive_i386_names[jive_i386_eax],
			.parent = &jive_root_register_class, .depth = 2,
			.priority = jive_resource_class_priority_reg_low,
			.demotions = (const jive_resource_class_demotion []) {
				{STACK4, VIA {CLS(gpr), STACK4, NULL}},
				{NULL, NULL}
			},
			.type = &bits32.base.base
		},
		.regs = &jive_i386_regs[jive_i386_eax],
		.nbits = 32, .int_arithmetic_width = 32, .loadstore_width = 8|16|32
	},
	[jive_i386_gpr_byte] = {
		.base = {
			.class_ = &JIVE_REGISTER_RESOURCE,
			.name = "gpr_byte_addressible",
			.limit = 4, .names = &jive_i386_names[jive_i386_eax],
			.parent = &jive_i386_regcls[jive_i386_gpr].base, .depth = 3,
			.priority = jive_resource_class_priority_reg_low,
			.demotions = (const jive_resource_class_demotion []) {
				{CLS(gpr), VIA {CLS(gpr), CLS(gpr), NULL}},
				{STACK4, VIA {CLS(gpr), STACK4, NULL}},
				{NULL, NULL}
			},
			.type = &bits32.base.base
		},
		.regs = &jive_i386_regs[jive_i386_eax],
		.nbits = 32, .int_arithmetic_width = 32, .loadstore_width = 8|16|32
	},
	[jive_i386_gpr_eax] = {
		.base = {
			.class_ = &JIVE_REGISTER_RESOURCE,
			.name = "gpr_eax",
			.limit = 1, .names = &jive_i386_names[jive_i386_eax],
			.parent = &jive_i386_regcls[jive_i386_gpr_byte].base, .depth = 4,
			.priority = jive_resource_class_priority_reg_low,
			.demotions = (const jive_resource_class_demotion []) {
				{CLS(gpr), VIA {CLS(gpr), CLS(gpr), NULL}},
				{STACK4, VIA {CLS(gpr), STACK4, NULL}},
				{NULL, NULL}
			},
			.type = &bits32.base.base
		},
		.regs = &jive_i386_regs[jive_i386_eax],
		.nbits = 32, .int_arithmetic_width = 32, .loadstore_width = 8|16|32
	},
	[jive_i386_gpr_ecx] = {
		.base = {
			.class_ = &JIVE_REGISTER_RESOURCE,
			.name = "gpr_ecx",
			.limit = 1, .names = &jive_i386_names[jive_i386_ecx],
			.parent = &jive_i386_regcls[jive_i386_gpr_byte].base, .depth = 4,
			.priority = jive_resource_class_priority_reg_low,
			.demotions = (const jive_resource_class_demotion []) {
				{CLS(gpr), VIA {CLS(gpr), CLS(gpr), NULL}},
				{STACK4, VIA {CLS(gpr), STACK4, NULL}},
				{NULL, NULL}
			},
			.type = &bits32.base.base
		},
		.regs = &jive_i386_regs[jive_i386_ecx],
		.nbits = 32, .int_arithmetic_width = 32, .loadstore_width = 8|16|32
	},
	[jive_i386_gpr_ebx] = {
		.base = {
			.class_ = &JIVE_REGISTER_RESOURCE,
			.name = "gpr_ebx",
			.limit = 1, .names = &jive_i386_names[jive_i386_ebx],
			.parent = &jive_i386_regcls[jive_i386_gpr_byte].base, .depth = 4,
			.priority = jive_resource_class_priority_reg_low,
			.demotions = (const jive_resource_class_demotion []) {
				{CLS(gpr), VIA {CLS(gpr), CLS(gpr), NULL}},
				{STACK4, VIA {CLS(gpr), STACK4, NULL}},
				{NULL, NULL}
			},
			.type = &bits32.base.base
		},
		.regs = &jive_i386_regs[jive_i386_ebx],
		.nbits = 32, .int_arithmetic_width = 32, .loadstore_width = 8|16|32
	},
	[jive_i386_gpr_edx] = {
		.base = {
			.class_ = &JIVE_REGISTER_RESOURCE,
			.name = "gpr_edx",
			.limit = 1, .names = &jive_i386_names[jive_i386_edx],
			.parent = &jive_i386_regcls[jive_i386_gpr_byte].base, .depth = 4,
			.priority = jive_resource_class_priority_reg_low,
			.demotions = (const jive_resource_class_demotion []) {
				{CLS(gpr), VIA {CLS(gpr), CLS(gpr), NULL}},
				{STACK4, VIA {CLS(gpr), STACK4, NULL}},
				{NULL, NULL}
			},
			.type = &bits32.base.base
		},
		.regs = &jive_i386_regs[jive_i386_edx],
		.nbits = 32, .int_arithmetic_width = 32, .loadstore_width = 8|16|32
	},
	[jive_i386_gpr_esi] = {
		.base = {
			.class_ = &JIVE_REGISTER_RESOURCE,
			.name = "gpr_esi",
			.limit = 1, .names = &jive_i386_names[jive_i386_esi],
			.parent = &jive_i386_regcls[jive_i386_gpr].base, .depth = 3,
			.priority = jive_resource_class_priority_reg_low,
			.demotions = (const jive_resource_class_demotion []) {
				{CLS(gpr), VIA {CLS(gpr), CLS(gpr), NULL}},
				{STACK4, VIA {CLS(gpr), STACK4, NULL}},
				{NULL, NULL}
			},
			.type = &bits32.base.base
		},
		.regs = &jive_i386_regs[jive_i386_esi],
		.nbits = 32, .int_arithmetic_width = 32, .loadstore_width = 8|16|32
	},
	[jive_i386_gpr_edi] = {
		.base = {
			.class_ = &JIVE_REGISTER_RESOURCE,
			.name = "gpr_edi",
			.limit = 1, .names = &jive_i386_names[jive_i386_edi],
			.parent = &jive_i386_regcls[jive_i386_gpr].base, .depth = 3,
			.priority = jive_resource_class_priority_reg_low,
			.demotions = (const jive_resource_class_demotion []) {
				{CLS(gpr), VIA {CLS(gpr), CLS(gpr), NULL}},
				{STACK4, VIA {CLS(gpr), STACK4, NULL}},
				{NULL, NULL}
			},
			.type = &bits32.base.base
		},
		.regs = &jive_i386_regs[jive_i386_edi],
		.nbits = 32, .int_arithmetic_width = 32, .loadstore_width = 8|16|32
	},
	[jive_i386_gpr_esp] = {
		.base = {
			.class_ = &JIVE_REGISTER_RESOURCE,
			.name = "gpr_esp",
			.limit = 1, .names = &jive_i386_names[jive_i386_esp],
			.parent = &jive_i386_regcls[jive_i386_gpr].base, .depth = 3,
			.priority = jive_resource_class_priority_reg_low,
			.demotions = (const jive_resource_class_demotion []) {
				{CLS(gpr), VIA {CLS(gpr), CLS(gpr), NULL}},
				{STACK4, VIA {CLS(gpr), STACK4, NULL}},
				{NULL, NULL}
			},
			.type = &bits32.base.base
		},
		.regs = &jive_i386_regs[jive_i386_esp],
		.nbits = 32, .int_arithmetic_width = 32, .loadstore_width = 8|16|32
	},
	[jive_i386_gpr_ebp] = {
		.base = {
			.class_ = &JIVE_REGISTER_RESOURCE,
			.name = "gpr_ebp",
			.limit = 1, .names = &jive_i386_names[jive_i386_ebp],
			.parent = &jive_i386_regcls[jive_i386_gpr].base, .depth = 3,
			.priority = jive_resource_class_priority_reg_low,
			.demotions = (const jive_resource_class_demotion []) {
				{CLS(gpr), VIA {CLS(gpr), CLS(gpr), NULL}},
				{STACK4, VIA {CLS(gpr), STACK4, NULL}},
				{NULL, NULL}
			},
			.type = &bits32.base.base
		},
		.regs = &jive_i386_regs[jive_i386_ebp],
		.nbits = 32, .int_arithmetic_width = 32, .loadstore_width = 8|16|32
	},

	[jive_i386_fp] = {
		.base = {
			.class_ = &JIVE_REGISTER_RESOURCE,
			.name = "fp",
			.limit = 1, .names = &jive_i386_names[jive_i386_st0],
			.parent = &jive_root_register_class, .depth = 2,
			.priority = jive_resource_class_priority_reg_low,
			.demotions = (const jive_resource_class_demotion []) {
				{STACK4, VIA {CLS(fp), STACK4, NULL}},
				{NULL, NULL}
			},
			.type = &flt.base.base
		},
		.regs = &jive_i386_regs[jive_i386_st0],
		.nbits = 80, .int_arithmetic_width = 80, .loadstore_width = 80
	},
	[jive_i386_fp_st0] = {
		.base = {
			.class_ = &JIVE_REGISTER_RESOURCE,
			.name = "fp_st0",
			.limit = 1, .names = &jive_i386_names[jive_i386_st0],
			.parent = &jive_i386_regcls[jive_i386_fp].base, .depth = 3,
			.priority = jive_resource_class_priority_reg_low,
			.demotions = (const jive_resource_class_demotion []) {
				{CLS(fp), VIA {CLS(fp), CLS(fp), NULL}},
				{STACK4, VIA {CLS(gpr), STACK4, NULL}},
				{NULL, NULL}
			},
			.type = &flt.base.base
		},
		.regs = &jive_i386_regs[jive_i386_st0],
		.nbits = 80, .int_arithmetic_width = 80, .loadstore_width = 80
	},
	
	[jive_i386_sse] = {
		.base = {
			.class_ = &JIVE_REGISTER_RESOURCE,
			.name = "sse",
			.limit = 8, .names = &jive_i386_names[jive_i386_xmm0],
			.parent = &jive_root_register_class, .depth = 2,
			.priority = jive_resource_class_priority_reg_low,
			.demotions = NULL, 
			.type = &flt.base.base
		},
		.regs = &jive_i386_regs[jive_i386_xmm0],
		.nbits = 32, .int_arithmetic_width = 128, .loadstore_width = 128
	},
	[jive_i386_sse_xmm0] = {
		.base = {
			.class_ = &JIVE_REGISTER_RESOURCE,
			.name = "sse_xmm0",
			.limit = 1, .names = &jive_i386_names[jive_i386_xmm0],
			.parent = &jive_i386_regcls[jive_i386_sse].base, .depth = 3,
			.priority = jive_resource_class_priority_reg_low,
			.demotions = NULL,
			.type = &flt.base.base
		},
		.regs = &jive_i386_regs[jive_i386_xmm0],
		.nbits = 32, .int_arithmetic_width = 128, .loadstore_width = 128
	},
	[jive_i386_sse_xmm1] = {
		.base = {
			.class_ = &JIVE_REGISTER_RESOURCE,
			.name = "sse_xmm1",
			.limit = 1, .names = &jive_i386_names[jive_i386_xmm1],
			.parent = &jive_i386_regcls[jive_i386_sse].base, .depth = 3,
			.priority = jive_resource_class_priority_reg_low,
			.demotions = NULL,
			.type = &flt.base.base
		},
		.regs = &jive_i386_regs[jive_i386_xmm1],
		.nbits = 32, .int_arithmetic_width = 128, .loadstore_width = 128
	},
	[jive_i386_sse_xmm2] = {
		.base = {
			.class_ = &JIVE_REGISTER_RESOURCE,
			.name = "sse_xmm2",
			.limit = 1, .names = &jive_i386_names[jive_i386_xmm2],
			.parent = &jive_i386_regcls[jive_i386_sse].base, .depth = 3,
			.priority = jive_resource_class_priority_reg_low,
			.demotions = NULL,
			.type = &flt.base.base
		},
		.regs = &jive_i386_regs[jive_i386_xmm2],
		.nbits = 32, .int_arithmetic_width = 128, .loadstore_width = 128
	},
	[jive_i386_sse_xmm3] = {
		.base = {
			.class_ = &JIVE_REGISTER_RESOURCE,
			.name = "sse_xmm3",
			.limit = 1, .names = &jive_i386_names[jive_i386_xmm3],
			.parent = &jive_i386_regcls[jive_i386_sse].base, .depth = 3,
			.priority = jive_resource_class_priority_reg_low,
			.demotions = NULL,
			.type = &flt.base.base
		},
		.regs = &jive_i386_regs[jive_i386_xmm3],
		.nbits = 32, .int_arithmetic_width = 128, .loadstore_width = 128
	},
	[jive_i386_sse_xmm4] = {
		.base = {
			.class_ = &JIVE_REGISTER_RESOURCE,
			.name = "sse_xmm4",
			.limit = 1, .names = &jive_i386_names[jive_i386_xmm4],
			.parent = &jive_i386_regcls[jive_i386_sse].base, .depth = 3,
			.priority = jive_resource_class_priority_reg_low,
			.demotions = NULL,
			.type = &flt.base.base
		},
		.regs = &jive_i386_regs[jive_i386_xmm4],
		.nbits = 32, .int_arithmetic_width = 128, .loadstore_width = 128
	},
	[jive_i386_sse_xmm5] = {
		.base = {
			.class_ = &JIVE_REGISTER_RESOURCE,
			.name = "sse_xmm5",
			.limit = 1, .names = &jive_i386_names[jive_i386_xmm5],
			.parent = &jive_i386_regcls[jive_i386_sse].base, .depth = 3,
			.priority = jive_resource_class_priority_reg_low,
			.demotions = NULL,
			.type = &flt.base.base
		},
		.regs = &jive_i386_regs[jive_i386_xmm5],
		.nbits = 32, .int_arithmetic_width = 128, .loadstore_width = 128
	},
	[jive_i386_sse_xmm6] = {
		.base = {
			.class_ = &JIVE_REGISTER_RESOURCE,
			.name = "sse_xmm6",
			.limit = 1, .names = &jive_i386_names[jive_i386_xmm6],
			.parent = &jive_i386_regcls[jive_i386_sse].base, .depth = 3,
			.priority = jive_resource_class_priority_reg_low,
			.demotions = NULL,
			.type = &flt.base.base
		},
		.regs = &jive_i386_regs[jive_i386_xmm6],
		.nbits = 32, .int_arithmetic_width = 128, .loadstore_width = 128
	},
	[jive_i386_sse_xmm7] = {
		.base = {
			.class_ = &JIVE_REGISTER_RESOURCE,
			.name = "sse_xmm7",
			.limit = 1, .names = &jive_i386_names[jive_i386_xmm7],
			.parent = &jive_i386_regcls[jive_i386_sse].base, .depth = 3,
			.priority = jive_resource_class_priority_reg_low,
			.demotions = NULL,
			.type = &flt.base.base
		},
		.regs = &jive_i386_regs[jive_i386_xmm7],
		.nbits = 32, .int_arithmetic_width = 128, .loadstore_width = 128
	},
};

JIVE_SERIALIZATION_REGCLSSET_REGISTER(jive_i386_regcls,
	sizeof(jive_i386_regcls) / sizeof(jive_i386_regcls[0]),
	"i386_");
