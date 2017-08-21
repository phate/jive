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

const jive_register_name jive_i386_reg_cc("cc", &jive_i386_regcls_flags, 0);
const jive_register_name jive_i386_reg_eax("eax", &jive_i386_regcls_gpr_eax, 0);
const jive_register_name jive_i386_reg_ecx("ecx", &jive_i386_regcls_gpr_ecx, 1);
const jive_register_name jive_i386_reg_edx("edx", &jive_i386_regcls_gpr_edx, 2);
const jive_register_name jive_i386_reg_ebx("ebx", &jive_i386_regcls_gpr_ebx, 3);
const jive_register_name jive_i386_reg_esi("esi", &jive_i386_regcls_gpr_esi, 6);
const jive_register_name jive_i386_reg_edi("edi", &jive_i386_regcls_gpr_edi, 7);
const jive_register_name jive_i386_reg_ebp("ebp", &jive_i386_regcls_gpr_ebp, 5);
const jive_register_name jive_i386_reg_esp("esp", &jive_i386_regcls_gpr_esp, 4);

const jive_register_name jive_i386_reg_st0("st0", &jive_i386_regcls_fp_st0, 0);

const jive_register_name jive_i386_reg_xmm0("xmm0", &jive_i386_regcls_sse_xmm0, 0);
const jive_register_name jive_i386_reg_xmm1("xmm1", &jive_i386_regcls_sse_xmm1, 1);
const jive_register_name jive_i386_reg_xmm2("xmm2", &jive_i386_regcls_sse_xmm2, 2);
const jive_register_name jive_i386_reg_xmm3("xmm3", &jive_i386_regcls_sse_xmm3, 3);
const jive_register_name jive_i386_reg_xmm4("xmm4", &jive_i386_regcls_sse_xmm4, 4);
const jive_register_name jive_i386_reg_xmm5("xmm5", &jive_i386_regcls_sse_xmm5, 5);
const jive_register_name jive_i386_reg_xmm6("xmm6", &jive_i386_regcls_sse_xmm6, 6);
const jive_register_name jive_i386_reg_xmm7("xmm7", &jive_i386_regcls_sse_xmm7, 7);

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

#define CLS(x) &jive_i386_regcls_##x
#define STACK4 &jive_stackslot_class_4_4
#define VIA (const jive_resource_class * const[])

static const jive::bits::type bits16(16);
static const jive::bits::type bits32(32);
static const jive::flt::type flt;
const jive_resource_class_demotion  tmparray0[] = {
			{CLS(gpr_eax), VIA {CLS(flags), CLS(gpr_eax), NULL}},
			{STACK4, VIA {CLS(flags), CLS(gpr_eax), STACK4, NULL}},
			{NULL, NULL}
		};
const jive_register_class jive_i386_regcls_flags(
	&JIVE_REGISTER_RESOURCE, "flags", 1,
	jive_i386_regcls_flags_names,
	&jive_root_register_class,
	jive_resource_class_priority_reg_high,
	tmparray0, &bits16, 16, 0, 0);

const jive_resource_class_demotion  tmparray1[] = {
			{STACK4, VIA {CLS(gpr), STACK4, NULL}},
			{NULL, NULL}
		};
const jive_register_class jive_i386_regcls_gpr(
	&JIVE_REGISTER_RESOURCE, "gpr", 8,
	jive_i386_regcls_gpr_names,
	&jive_root_register_class,
	jive_resource_class_priority_reg_low,
	tmparray1, &bits32, 32, 32, 8|16|32);

const jive_resource_class_demotion  tmparray2[] = {
			{CLS(gpr), VIA {CLS(gpr), CLS(gpr), NULL}},
			{STACK4, VIA {CLS(gpr), STACK4, NULL}},
			{NULL, NULL}
		};
const jive_register_class jive_i386_regcls_gpr_byte(
	&JIVE_REGISTER_RESOURCE, "gpr_byte_addressible", 4,
	jive_i386_regcls_gpr_byte_names,
	&jive_i386_regcls_gpr,
	jive_resource_class_priority_reg_low,
	tmparray2, &bits32, 32, 32, 8|16|32);

const jive_resource_class_demotion  tmparray3[] = {
			{CLS(gpr), VIA {CLS(gpr), CLS(gpr), NULL}},
			{STACK4, VIA {CLS(gpr), STACK4, NULL}},
			{NULL, NULL}
		};
const jive_register_class jive_i386_regcls_gpr_eax(
	&JIVE_REGISTER_RESOURCE, "gpr_eax", 1,
	jive_i386_regcls_eax_names,
	&jive_i386_regcls_gpr_byte,
	jive_resource_class_priority_reg_low,
	tmparray3, &bits32, 32, 32, 8|16|32);

const jive_resource_class_demotion  tmparray4[] = {
			{CLS(gpr), VIA {CLS(gpr), CLS(gpr), NULL}},
			{STACK4, VIA {CLS(gpr), STACK4, NULL}},
			{NULL, NULL}
		};
const jive_register_class jive_i386_regcls_gpr_ecx(
	&JIVE_REGISTER_RESOURCE, "gpr_ecx", 1,
	jive_i386_regcls_ecx_names,
	&jive_i386_regcls_gpr_byte,
	jive_resource_class_priority_reg_low,
	tmparray4, &bits32, 32, 32, 8|16|32);

const jive_resource_class_demotion  tmparray5[] = {
			{CLS(gpr), VIA {CLS(gpr), CLS(gpr), NULL}},
			{STACK4, VIA {CLS(gpr), STACK4, NULL}},
			{NULL, NULL}
		};
const jive_register_class jive_i386_regcls_gpr_ebx(
	&JIVE_REGISTER_RESOURCE, "gpr_ebx", 1,
	jive_i386_regcls_ebx_names,
	&jive_i386_regcls_gpr_byte,
	jive_resource_class_priority_reg_low,
	tmparray5, &bits32, 32, 32, 8|16|32);

const jive_resource_class_demotion  tmparray6[] = {
			{CLS(gpr), VIA {CLS(gpr), CLS(gpr), NULL}},
			{STACK4, VIA {CLS(gpr), STACK4, NULL}},
			{NULL, NULL}
		};
const jive_register_class jive_i386_regcls_gpr_edx(
	&JIVE_REGISTER_RESOURCE, "gpr_edx", 1,
	jive_i386_regcls_edx_names,
	&jive_i386_regcls_gpr_byte,
	jive_resource_class_priority_reg_low,
	tmparray6, &bits32, 32, 32, 8|16|32);

const jive_resource_class_demotion  tmparray7[] = {
			{CLS(gpr), VIA {CLS(gpr), CLS(gpr), NULL}},
			{STACK4, VIA {CLS(gpr), STACK4, NULL}},
			{NULL, NULL}
		};
const jive_register_class jive_i386_regcls_gpr_esi(
	&JIVE_REGISTER_RESOURCE, "gpr_esi", 1,
	jive_i386_regcls_esi_names,
	&jive_i386_regcls_gpr,
	jive_resource_class_priority_reg_low,
	tmparray7, &bits32, 32, 32, 8|16|32);

const jive_resource_class_demotion  tmparray8[] = {
			{CLS(gpr), VIA {CLS(gpr), CLS(gpr), NULL}},
			{STACK4, VIA {CLS(gpr), STACK4, NULL}},
			{NULL, NULL}
		};
const jive_register_class jive_i386_regcls_gpr_edi(
	&JIVE_REGISTER_RESOURCE, "gpr_edi", 1,
	jive_i386_regcls_edi_names,
	&jive_i386_regcls_gpr,
	jive_resource_class_priority_reg_low,
	tmparray8, &bits32, 32, 32, 8|16|32);

const jive_resource_class_demotion  tmparray9[] = {
			{CLS(gpr), VIA {CLS(gpr), CLS(gpr), NULL}},
			{STACK4, VIA {CLS(gpr), STACK4, NULL}},
			{NULL, NULL}
		};
const jive_register_class jive_i386_regcls_gpr_esp(
	&JIVE_REGISTER_RESOURCE, "gpr_esp", 1,
	jive_i386_regcls_esp_names,
	&jive_i386_regcls_gpr,
	jive_resource_class_priority_reg_low,
	tmparray9, &bits32, 32, 32, 8|16|32);

const jive_resource_class_demotion  tmparray10[] = {
			{CLS(gpr), VIA {CLS(gpr), CLS(gpr), NULL}},
			{STACK4, VIA {CLS(gpr), STACK4, NULL}},
			{NULL, NULL}
		};
const jive_register_class jive_i386_regcls_gpr_ebp(
	&JIVE_REGISTER_RESOURCE, "gpr_ebp", 1,
	jive_i386_regcls_ebp_names,
	&jive_i386_regcls_gpr,
	jive_resource_class_priority_reg_low,
	tmparray10, &bits32, 32, 32, 8|16|32);

const jive_resource_class_demotion  tmparray11[] = {
			{STACK4, VIA {CLS(fp), STACK4, NULL}},
			{NULL, NULL}
		};
const jive_register_class jive_i386_regcls_fp(
	&JIVE_REGISTER_RESOURCE, "fp", 1,
	jive_i386_regcls_fp_names,
	&jive_root_register_class,
	jive_resource_class_priority_reg_low,
	tmparray11, &flt, 80, 80, 80);

const jive_resource_class_demotion  tmparray12[] = {
			{CLS(fp), VIA {CLS(fp), CLS(fp), NULL}},
			{STACK4, VIA {CLS(gpr), STACK4, NULL}},
			{NULL, NULL}
		};
const jive_register_class jive_i386_regcls_fp_st0(
	&JIVE_REGISTER_RESOURCE, "fp_st0", 1,
	jive_i386_regcls_fp_names,
	&jive_i386_regcls_fp,
	jive_resource_class_priority_reg_low,
	tmparray12, &flt, 80, 80, 80);

const jive_register_class jive_i386_regcls_sse(
	&JIVE_REGISTER_RESOURCE, "sse", 8,
	jive_i386_regcls_sse_names,
	&jive_root_register_class,
	jive_resource_class_priority_reg_low,
	nullptr, &flt, 32, 128, 128);

const jive_register_class jive_i386_regcls_sse_xmm0(
	&JIVE_REGISTER_RESOURCE, "sse_xmm0", 1,
	jive_i386_regcls_sse_xmm0_names,
	&jive_i386_regcls_sse,
	jive_resource_class_priority_reg_low,
	nullptr, &flt, 32, 128, 128);

const jive_register_class jive_i386_regcls_sse_xmm1(
	&JIVE_REGISTER_RESOURCE, "sse_xmm1", 1,
	jive_i386_regcls_sse_xmm1_names,
	&jive_i386_regcls_sse,
	jive_resource_class_priority_reg_low,
	nullptr, &flt, 32, 128, 128);

const jive_register_class jive_i386_regcls_sse_xmm2(
	&JIVE_REGISTER_RESOURCE, "sse_xmm2", 1,
	jive_i386_regcls_sse_xmm2_names,
	&jive_i386_regcls_sse,
	jive_resource_class_priority_reg_low,
	nullptr, &flt, 32, 128, 128);

const jive_register_class jive_i386_regcls_sse_xmm3(
	&JIVE_REGISTER_RESOURCE, "sse_xmm3", 1,
	jive_i386_regcls_sse_xmm3_names,
	&jive_i386_regcls_sse,
	jive_resource_class_priority_reg_low,
	nullptr, &flt, 32, 128, 128);

const jive_register_class jive_i386_regcls_sse_xmm4(
	&JIVE_REGISTER_RESOURCE, "sse_xmm4", 1,
	jive_i386_regcls_sse_xmm4_names,
	&jive_i386_regcls_sse,
	jive_resource_class_priority_reg_low,
	nullptr, &flt, 32, 128, 128);

const jive_register_class jive_i386_regcls_sse_xmm5(
	&JIVE_REGISTER_RESOURCE, "sse_xmm5", 1,
	jive_i386_regcls_sse_xmm5_names,
	&jive_i386_regcls_sse,
	jive_resource_class_priority_reg_low,
	nullptr, &flt, 32, 128, 128);

const jive_register_class jive_i386_regcls_sse_xmm6(
	&JIVE_REGISTER_RESOURCE, "sse_xmm6", 1,
	jive_i386_regcls_sse_xmm6_names,
	&jive_i386_regcls_sse,
	jive_resource_class_priority_reg_low,
	nullptr, &flt, 32, 128, 128);

const jive_register_class jive_i386_regcls_sse_xmm7(
	&JIVE_REGISTER_RESOURCE, "sse_xmm7", 1,
	jive_i386_regcls_sse_xmm7_names,
	&jive_i386_regcls_sse,
	jive_resource_class_priority_reg_low,
	nullptr, &flt, 32, 128, 128);
