/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/backend/i386/registerset.h>

#include <jive/arch/registers.h>
#include <jive/arch/stackslot.h>
#include <jive/types/bitstring/type.h>
#include <jive/types/float/flttype.h>

const jive_register_name jive_i386_reg_cc("cc", &jive_i386_regcls_flags.base, 0);
const jive_register_name jive_i386_reg_eax("eax", &jive_i386_regcls_gpr_eax.base, 0);
const jive_register_name jive_i386_reg_ecx("ecx", &jive_i386_regcls_gpr_ecx.base, 1);
const jive_register_name jive_i386_reg_edx("edx", &jive_i386_regcls_gpr_edx.base, 2);
const jive_register_name jive_i386_reg_ebx("ebx", &jive_i386_regcls_gpr_ebx.base, 3);
const jive_register_name jive_i386_reg_esi("esi", &jive_i386_regcls_gpr_esi.base, 6);
const jive_register_name jive_i386_reg_edi("edi", &jive_i386_regcls_gpr_edi.base, 7);
const jive_register_name jive_i386_reg_ebp("ebp", &jive_i386_regcls_gpr_ebp.base, 5);
const jive_register_name jive_i386_reg_esp("esp", &jive_i386_regcls_gpr_esp.base, 4);

const jive_register_name jive_i386_reg_st0("st0", &jive_i386_regcls_fp_st0.base, 0);

const jive_register_name jive_i386_reg_xmm0("xmm0", &jive_i386_regcls_sse_xmm0.base, 0);
const jive_register_name jive_i386_reg_xmm1("xmm1", &jive_i386_regcls_sse_xmm1.base, 1);
const jive_register_name jive_i386_reg_xmm2("xmm2", &jive_i386_regcls_sse_xmm2.base, 2);
const jive_register_name jive_i386_reg_xmm3("xmm3", &jive_i386_regcls_sse_xmm3.base, 3);
const jive_register_name jive_i386_reg_xmm4("xmm4", &jive_i386_regcls_sse_xmm4.base, 4);
const jive_register_name jive_i386_reg_xmm5("xmm5", &jive_i386_regcls_sse_xmm5.base, 5);
const jive_register_name jive_i386_reg_xmm6("xmm6", &jive_i386_regcls_sse_xmm6.base, 6);
const jive_register_name jive_i386_reg_xmm7("xmm7", &jive_i386_regcls_sse_xmm7.base, 7);

static const jive_resource_name * jive_i386_regcls_flags_names [] = {&jive_i386_reg_cc};

static const jive_resource_name * jive_i386_regcls_gpr_names [] = {
	&jive_i386_reg_eax, &jive_i386_reg_ecx,
	&jive_i386_reg_ebx, &jive_i386_reg_edx,
	&jive_i386_reg_esi, &jive_i386_reg_edi,
	&jive_i386_reg_ebp, &jive_i386_reg_esp,
};

static const jive_resource_name * jive_i386_regcls_gpr_byte_names [] = {
	&jive_i386_reg_eax, &jive_i386_reg_ecx,
	&jive_i386_reg_ebx, &jive_i386_reg_edx,
};

static const jive_resource_name * jive_i386_regcls_eax_names [] = {&jive_i386_reg_eax};
static const jive_resource_name * jive_i386_regcls_ecx_names [] = {&jive_i386_reg_ecx};
static const jive_resource_name * jive_i386_regcls_ebx_names [] = {&jive_i386_reg_ebx};
static const jive_resource_name * jive_i386_regcls_edx_names [] = {&jive_i386_reg_edx};
static const jive_resource_name * jive_i386_regcls_esi_names [] = {&jive_i386_reg_esi};
static const jive_resource_name * jive_i386_regcls_edi_names [] = {&jive_i386_reg_edi};
static const jive_resource_name * jive_i386_regcls_ebp_names [] = {&jive_i386_reg_ebp};
static const jive_resource_name * jive_i386_regcls_esp_names [] = {&jive_i386_reg_esp};

static const jive_resource_name * jive_i386_regcls_fp_names [] = {&jive_i386_reg_st0};

static const jive_resource_name * jive_i386_regcls_sse_names [] = {
	&jive_i386_reg_xmm0, &jive_i386_reg_xmm1,
	&jive_i386_reg_xmm2, &jive_i386_reg_xmm3,
	&jive_i386_reg_xmm4, &jive_i386_reg_xmm5,
	&jive_i386_reg_xmm6, &jive_i386_reg_xmm7
};
static const jive_resource_name * jive_i386_regcls_sse_xmm0_names [] = {&jive_i386_reg_xmm0};
static const jive_resource_name * jive_i386_regcls_sse_xmm1_names [] = {&jive_i386_reg_xmm1};
static const jive_resource_name * jive_i386_regcls_sse_xmm2_names [] = {&jive_i386_reg_xmm2};
static const jive_resource_name * jive_i386_regcls_sse_xmm3_names [] = {&jive_i386_reg_xmm3};
static const jive_resource_name * jive_i386_regcls_sse_xmm4_names [] = {&jive_i386_reg_xmm4};
static const jive_resource_name * jive_i386_regcls_sse_xmm5_names [] = {&jive_i386_reg_xmm5};
static const jive_resource_name * jive_i386_regcls_sse_xmm6_names [] = {&jive_i386_reg_xmm6};
static const jive_resource_name * jive_i386_regcls_sse_xmm7_names [] = {&jive_i386_reg_xmm7};

#define CLS(x) &jive_i386_regcls_##x.base
#define STACK4 &jive_stackslot_class_4_4.base
#define VIA (const jive_resource_class * const[])

static const jive::bits::type bits16(16);
static const jive::bits::type bits32(32);
static const jive::flt::type flt;
const jive_resource_class_demotion  tmparray0[] = {
			{CLS(gpr_eax), VIA {CLS(flags), CLS(gpr_eax), NULL}},
			{STACK4, VIA {CLS(flags), CLS(gpr_eax), STACK4, NULL}},
			{NULL, NULL}
		};

const jive_register_class jive_i386_regcls_flags = {
	base : {
		class_ : &JIVE_REGISTER_RESOURCE,
		name : "flags",
		limit : 1, names : jive_i386_regcls_flags_names,
		parent : &jive_root_register_class, depth : 2,
		priority : jive_resource_class_priority_reg_high,
		demotions : tmparray0,
		type : &bits16
	},
	nbits : 16
};
const jive_resource_class_demotion  tmparray1[] = {
			{STACK4, VIA {CLS(gpr), STACK4, NULL}},
			{NULL, NULL}
		};
const jive_register_class jive_i386_regcls_gpr = {
	base : {
		class_ : &JIVE_REGISTER_RESOURCE,
		name : "gpr",
		limit : 8, names : jive_i386_regcls_gpr_names,
		parent : &jive_root_register_class, depth : 2,
		priority : jive_resource_class_priority_reg_low,
		demotions : tmparray1,
		type : &bits32
	},
	nbits : 32, int_arithmetic_width : 32, loadstore_width : 8|16|32
};
const jive_resource_class_demotion  tmparray2[] = {
			{CLS(gpr), VIA {CLS(gpr), CLS(gpr), NULL}},
			{STACK4, VIA {CLS(gpr), STACK4, NULL}},
			{NULL, NULL}
		};
const jive_register_class jive_i386_regcls_gpr_byte = {
	base : {
		class_ : &JIVE_REGISTER_RESOURCE,
		name : "gpr_byte_addressible",
		limit : 4, names : jive_i386_regcls_gpr_byte_names,
		parent : &jive_i386_regcls_gpr.base, depth : 3,
		priority : jive_resource_class_priority_reg_low,
		demotions : tmparray2,
		type : &bits32
	},
	nbits : 32, int_arithmetic_width : 32, loadstore_width : 8|16|32
};
const jive_resource_class_demotion  tmparray3[] = {
			{CLS(gpr), VIA {CLS(gpr), CLS(gpr), NULL}},
			{STACK4, VIA {CLS(gpr), STACK4, NULL}},
			{NULL, NULL}
		};
const jive_register_class jive_i386_regcls_gpr_eax = {
	base : {
		class_ : &JIVE_REGISTER_RESOURCE,
		name : "gpr_eax",
		limit : 1, names : jive_i386_regcls_eax_names,
		parent : &jive_i386_regcls_gpr_byte.base, depth : 4,
		priority : jive_resource_class_priority_reg_low,
		demotions : tmparray3,
		type : &bits32
	},
	nbits : 32, int_arithmetic_width : 32, loadstore_width : 8|16|32
};
const jive_resource_class_demotion  tmparray4[] = {
			{CLS(gpr), VIA {CLS(gpr), CLS(gpr), NULL}},
			{STACK4, VIA {CLS(gpr), STACK4, NULL}},
			{NULL, NULL}
		};
const jive_register_class jive_i386_regcls_gpr_ecx = {
	base : {
		class_ : &JIVE_REGISTER_RESOURCE,
		name : "gpr_ecx",
		limit : 1, names : jive_i386_regcls_ecx_names,
		parent : &jive_i386_regcls_gpr_byte.base, depth : 4,
		priority : jive_resource_class_priority_reg_low,
		demotions : tmparray4,
		type : &bits32
	},
	nbits : 32, int_arithmetic_width : 32, loadstore_width : 8|16|32
};
const jive_resource_class_demotion  tmparray5[] = {
			{CLS(gpr), VIA {CLS(gpr), CLS(gpr), NULL}},
			{STACK4, VIA {CLS(gpr), STACK4, NULL}},
			{NULL, NULL}
		};
const jive_register_class jive_i386_regcls_gpr_ebx = {
	base : {
		class_ : &JIVE_REGISTER_RESOURCE,
		name : "gpr_ebx",
		limit : 1, names : jive_i386_regcls_ebx_names,
		parent : &jive_i386_regcls_gpr_byte.base, depth : 4,
		priority : jive_resource_class_priority_reg_low,
		demotions : tmparray5,
		type : &bits32
	},
	nbits : 32, int_arithmetic_width : 32, loadstore_width : 8|16|32
};
const jive_resource_class_demotion  tmparray6[] = {
			{CLS(gpr), VIA {CLS(gpr), CLS(gpr), NULL}},
			{STACK4, VIA {CLS(gpr), STACK4, NULL}},
			{NULL, NULL}
		};
const jive_register_class jive_i386_regcls_gpr_edx = {
	base : {
		class_ : &JIVE_REGISTER_RESOURCE,
		name : "gpr_edx",
		limit : 1, jive_i386_regcls_edx_names,
		parent : &jive_i386_regcls_gpr_byte.base, depth : 4,
		priority : jive_resource_class_priority_reg_low,
		demotions : tmparray6,
		type : &bits32
	},
	nbits : 32, int_arithmetic_width : 32, loadstore_width : 8|16|32
};
const jive_resource_class_demotion  tmparray7[] = {
			{CLS(gpr), VIA {CLS(gpr), CLS(gpr), NULL}},
			{STACK4, VIA {CLS(gpr), STACK4, NULL}},
			{NULL, NULL}
		};
const jive_register_class jive_i386_regcls_gpr_esi = {
	base : {
		class_ : &JIVE_REGISTER_RESOURCE,
		name : "gpr_esi",
		limit : 1, names : jive_i386_regcls_esi_names,
		parent : &jive_i386_regcls_gpr.base, depth : 3,
		priority : jive_resource_class_priority_reg_low,
		demotions : tmparray7,
		type : &bits32
	},
	nbits : 32, int_arithmetic_width : 32, loadstore_width : 8|16|32
};
const jive_resource_class_demotion  tmparray8[] = {
			{CLS(gpr), VIA {CLS(gpr), CLS(gpr), NULL}},
			{STACK4, VIA {CLS(gpr), STACK4, NULL}},
			{NULL, NULL}
		};
const jive_register_class jive_i386_regcls_gpr_edi = {
	base : {
		class_ : &JIVE_REGISTER_RESOURCE,
		name : "gpr_edi",
		limit : 1, names : jive_i386_regcls_edi_names,
		parent : &jive_i386_regcls_gpr.base, depth : 3,
		priority : jive_resource_class_priority_reg_low,
		demotions : tmparray8,
		type : &bits32
	},
	nbits : 32, int_arithmetic_width : 32, loadstore_width : 8|16|32
};
const jive_resource_class_demotion  tmparray9[] = {
			{CLS(gpr), VIA {CLS(gpr), CLS(gpr), NULL}},
			{STACK4, VIA {CLS(gpr), STACK4, NULL}},
			{NULL, NULL}
		};
const jive_register_class jive_i386_regcls_gpr_esp = {
	base : {
		class_ : &JIVE_REGISTER_RESOURCE,
		name : "gpr_esp",
		limit : 1, names : jive_i386_regcls_esp_names,
		parent : &jive_i386_regcls_gpr.base, depth : 3,
		priority : jive_resource_class_priority_reg_low,
		demotions : tmparray9,
		type : &bits32
	},
	nbits : 32, int_arithmetic_width : 32, loadstore_width : 8|16|32
};
const jive_resource_class_demotion  tmparray10[] = {
			{CLS(gpr), VIA {CLS(gpr), CLS(gpr), NULL}},
			{STACK4, VIA {CLS(gpr), STACK4, NULL}},
			{NULL, NULL}
		};
const jive_register_class jive_i386_regcls_gpr_ebp = {
	base : {
		class_ : &JIVE_REGISTER_RESOURCE,
		name : "gpr_ebp",
		limit : 1, names : jive_i386_regcls_ebp_names,
		parent : &jive_i386_regcls_gpr.base, depth : 3,
		priority : jive_resource_class_priority_reg_low,
		demotions : tmparray10,
		type : &bits32
	},
	nbits : 32, int_arithmetic_width : 32, loadstore_width : 8|16|32
};
const jive_resource_class_demotion  tmparray11[] = {
			{STACK4, VIA {CLS(fp), STACK4, NULL}},
			{NULL, NULL}
		};

const jive_register_class jive_i386_regcls_fp = {
	base : {
		class_ : &JIVE_REGISTER_RESOURCE,
		name : "fp",
		limit : 1, names : jive_i386_regcls_fp_names,
		parent : &jive_root_register_class, depth : 2,
		priority : jive_resource_class_priority_reg_low,
		demotions : tmparray11,
		type : &flt
	},
	nbits : 80, int_arithmetic_width : 80, loadstore_width : 80
};
const jive_resource_class_demotion  tmparray12[] = {
			{CLS(fp), VIA {CLS(fp), CLS(fp), NULL}},
			{STACK4, VIA {CLS(gpr), STACK4, NULL}},
			{NULL, NULL}
		};
const jive_register_class jive_i386_regcls_fp_st0 = {
	base : {
		class_ : &JIVE_REGISTER_RESOURCE,
		name : "fp_st0",
		limit : 1, names : jive_i386_regcls_fp_names,
		parent : &jive_i386_regcls_fp.base, depth : 3,
		priority : jive_resource_class_priority_reg_low,
		demotions : tmparray12,
		type : &flt
	},
	nbits : 80, int_arithmetic_width : 80, loadstore_width : 80
};

const jive_register_class jive_i386_regcls_sse = {
	base : {
		class_ : &JIVE_REGISTER_RESOURCE,
		name : "sse",
		limit : 8, names : jive_i386_regcls_sse_names,
		parent : &jive_root_register_class, depth : 2,
		priority : jive_resource_class_priority_reg_low,
		demotions : NULL,
		type : &flt
	},
	nbits : 32, int_arithmetic_width : 128, loadstore_width : 128
};
const jive_register_class jive_i386_regcls_sse_xmm0 = {
	base : {
		class_ : &JIVE_REGISTER_RESOURCE,
		name : "sse_xmm0",
		limit : 1, names : jive_i386_regcls_sse_xmm0_names,
		parent : &jive_i386_regcls_sse.base, depth : 3,
		priority : jive_resource_class_priority_reg_low,
		demotions : NULL,
		type : &flt
	},
	nbits : 32, int_arithmetic_width : 128, loadstore_width : 128
};
const jive_register_class jive_i386_regcls_sse_xmm1 = {
	base : {
		class_ : &JIVE_REGISTER_RESOURCE,
		name : "sse_xmm1",
		limit : 1, names : jive_i386_regcls_sse_xmm1_names,
		parent : &jive_i386_regcls_sse.base, depth : 3,
		priority : jive_resource_class_priority_reg_low,
		demotions : NULL,
		type : &flt
	},
	nbits : 32, int_arithmetic_width : 128, loadstore_width : 128
};
const jive_register_class jive_i386_regcls_sse_xmm2 = {
	base : {
		class_ : &JIVE_REGISTER_RESOURCE,
		name : "sse_xmm2",
		limit : 1, names : jive_i386_regcls_sse_xmm2_names,
		parent : &jive_i386_regcls_sse.base, depth : 3,
		priority : jive_resource_class_priority_reg_low,
		demotions : NULL,
		type : &flt
	},
	nbits : 32, int_arithmetic_width : 128, loadstore_width : 128
};
const jive_register_class jive_i386_regcls_sse_xmm3 = {
	base : {
		class_ : &JIVE_REGISTER_RESOURCE,
		name : "sse_xmm3",
		limit : 1, names : jive_i386_regcls_sse_xmm3_names,
		parent : &jive_i386_regcls_sse.base, depth : 3,
		priority : jive_resource_class_priority_reg_low,
		demotions : NULL,
		type : &flt
	},
	nbits : 32, int_arithmetic_width : 128, loadstore_width : 128
};
const jive_register_class jive_i386_regcls_sse_xmm4 = {
	base : {
		class_ : &JIVE_REGISTER_RESOURCE,
		name : "sse_xmm4",
		limit : 1, names : jive_i386_regcls_sse_xmm4_names,
		parent : &jive_i386_regcls_sse.base, depth : 3,
		priority : jive_resource_class_priority_reg_low,
		demotions : NULL,
		type : &flt
	},
	nbits : 32, int_arithmetic_width : 128, loadstore_width : 128
};
const jive_register_class jive_i386_regcls_sse_xmm5 = {
	base : {
		class_ : &JIVE_REGISTER_RESOURCE,
		name : "sse_xmm5",
		limit : 1, names : jive_i386_regcls_sse_xmm5_names,
		parent : &jive_i386_regcls_sse.base, depth : 3,
		priority : jive_resource_class_priority_reg_low,
		demotions : NULL,
		type : &flt
	},
	nbits : 32, int_arithmetic_width : 128, loadstore_width : 128
};
const jive_register_class jive_i386_regcls_sse_xmm6 = {
	base : {
		class_ : &JIVE_REGISTER_RESOURCE,
		name : "sse_xmm6",
		limit : 1, names : jive_i386_regcls_sse_xmm6_names,
		parent : &jive_i386_regcls_sse.base, depth : 3,
		priority : jive_resource_class_priority_reg_low,
		demotions : NULL,
		type : &flt
	},
	nbits : 32, int_arithmetic_width : 128, loadstore_width : 128
};
const jive_register_class jive_i386_regcls_sse_xmm7 = {
	base : {
		class_ : &JIVE_REGISTER_RESOURCE,
		name : "sse_xmm7",
		limit : 1, names : jive_i386_regcls_sse_xmm7_names,
		parent : &jive_i386_regcls_sse.base, depth : 3,
		priority : jive_resource_class_priority_reg_low,
		demotions : NULL,
		type : &flt
	},
	nbits : 32, int_arithmetic_width : 128, loadstore_width : 128
};
