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

namespace jive {
namespace i386 {

const jive::registers reg_cc("cc", &regcls_flags, 0);
const jive::registers reg_eax("eax", &regcls_gpr_eax, 0);
const jive::registers reg_ecx("ecx", &regcls_gpr_ecx, 1);
const jive::registers reg_edx("edx", &regcls_gpr_edx, 2);
const jive::registers reg_ebx("ebx", &regcls_gpr_ebx, 3);
const jive::registers reg_esi("esi", &regcls_gpr_esi, 6);
const jive::registers reg_edi("edi", &regcls_gpr_edi, 7);
const jive::registers reg_ebp("ebp", &regcls_gpr_ebp, 5);
const jive::registers reg_esp("esp", &regcls_gpr_esp, 4);

const jive::registers reg_st0("st0", &regcls_fp_st0, 0);

const jive::registers reg_xmm0("xmm0", &regcls_sse_xmm0, 0);
const jive::registers reg_xmm1("xmm1", &regcls_sse_xmm1, 1);
const jive::registers reg_xmm2("xmm2", &regcls_sse_xmm2, 2);
const jive::registers reg_xmm3("xmm3", &regcls_sse_xmm3, 3);
const jive::registers reg_xmm4("xmm4", &regcls_sse_xmm4, 4);
const jive::registers reg_xmm5("xmm5", &regcls_sse_xmm5, 5);
const jive::registers reg_xmm6("xmm6", &regcls_sse_xmm6, 6);
const jive::registers reg_xmm7("xmm7", &regcls_sse_xmm7, 7);

#define CLS(x) &regcls_##x
#define STACK4 &jive_stackslot_class_4_4

static const jive::bits::type bits16(16);
static const jive::bits::type bits32(32);
static const jive::flt::type flt;

const jive::register_class regcls_flags(
	"flags", {&reg_cc}, &jive_root_register_class, jive_resource_class_priority_reg_high,
	{{CLS(gpr_eax), {CLS(flags), CLS(gpr_eax)}}, {STACK4, {CLS(flags), CLS(gpr_eax), STACK4}}},
	&bits16, 16, 0, 0);

const jive::register_class regcls_gpr(
	"gpr", {&reg_eax, &reg_ecx, &reg_ebx, &reg_edx, &reg_esi, &reg_edi, &reg_ebp, &reg_esp},
	&jive_root_register_class, jive_resource_class_priority_reg_low,
	{{STACK4, {CLS(gpr), STACK4}}}, &bits32, 32, 32, 8|16|32);

const jive::register_class regcls_gpr_byte(
	"gpr_byte_addressible", {&reg_eax, &reg_ecx, &reg_ebx, &reg_edx},
	&regcls_gpr, jive_resource_class_priority_reg_low,
	{{CLS(gpr), {CLS(gpr), CLS(gpr)}}, {STACK4, {CLS(gpr), STACK4}}},
	&bits32, 32, 32, 8|16|32);

const jive::register_class regcls_gpr_eax(
	"gpr_eax", {&reg_eax}, &regcls_gpr_byte, jive_resource_class_priority_reg_low,
	{{CLS(gpr), {CLS(gpr), CLS(gpr)}}, {STACK4, {CLS(gpr), STACK4}}},
	&bits32, 32, 32, 8|16|32);

const jive::register_class regcls_gpr_ecx(
	"gpr_ecx", {&reg_ecx}, &regcls_gpr_byte, jive_resource_class_priority_reg_low,
	{{CLS(gpr), {CLS(gpr), CLS(gpr)}}, {STACK4, {CLS(gpr), STACK4}}},
	&bits32, 32, 32, 8|16|32);

const jive::register_class regcls_gpr_ebx(
	"gpr_ebx", {&reg_ebx}, &regcls_gpr_byte, jive_resource_class_priority_reg_low,
	{{CLS(gpr), {CLS(gpr), CLS(gpr)}}, {STACK4, {CLS(gpr), STACK4}}},
	&bits32, 32, 32, 8|16|32);

const jive::register_class regcls_gpr_edx(
	"gpr_edx", {&reg_edx}, &regcls_gpr_byte, jive_resource_class_priority_reg_low,
	{{CLS(gpr), {CLS(gpr), CLS(gpr)}}, {STACK4, {CLS(gpr), STACK4}}},
	&bits32, 32, 32, 8|16|32);

const jive::register_class regcls_gpr_esi(
	"gpr_esi", {&reg_esi}, &regcls_gpr, jive_resource_class_priority_reg_low,
	{{CLS(gpr), {CLS(gpr), CLS(gpr)}}, {STACK4, {CLS(gpr), STACK4}}},
	&bits32, 32, 32, 8|16|32);

const jive::register_class regcls_gpr_edi(
	"gpr_edi", {&reg_edi}, &regcls_gpr, jive_resource_class_priority_reg_low,
	{{CLS(gpr), {CLS(gpr), CLS(gpr)}}, {STACK4, {CLS(gpr), STACK4}}},
	&bits32, 32, 32, 8|16|32);

const jive::register_class regcls_gpr_esp(
	"gpr_esp", {&reg_esp}, &regcls_gpr, jive_resource_class_priority_reg_low,
	{{CLS(gpr), {CLS(gpr), CLS(gpr)}}, {STACK4, {CLS(gpr), STACK4}}},
	&bits32, 32, 32, 8|16|32);

const jive::register_class regcls_gpr_ebp(
	"gpr_ebp", {&reg_ebp}, &regcls_gpr, jive_resource_class_priority_reg_low,
	{{CLS(gpr), {CLS(gpr), CLS(gpr)}}, {STACK4, {CLS(gpr), STACK4}}},
	&bits32, 32, 32, 8|16|32);

const jive::register_class regcls_fp(
	"fp", {&reg_st0}, &jive_root_register_class, jive_resource_class_priority_reg_low,
	{{STACK4, {CLS(fp), STACK4}}}, &flt, 80, 80, 80);

const jive::register_class regcls_fp_st0(
	"fp_st0", {&reg_st0}, &regcls_fp, jive_resource_class_priority_reg_low,
	{{CLS(fp), {CLS(fp), CLS(fp)}}, {STACK4, {CLS(gpr), STACK4}}},
	&flt, 80, 80, 80);

const jive::register_class regcls_sse(
	"sse", {&reg_xmm0, &reg_xmm1, &reg_xmm2, &reg_xmm3, &reg_xmm4, &reg_xmm5, &reg_xmm6, &reg_xmm7},
	&jive_root_register_class, jive_resource_class_priority_reg_low,
	{}, &flt, 32, 128, 128);

const jive::register_class regcls_sse_xmm0(
	"sse_xmm0", {&reg_xmm0}, &regcls_sse, jive_resource_class_priority_reg_low,
	{}, &flt, 32, 128, 128);

const jive::register_class regcls_sse_xmm1(
	"sse_xmm1", {&reg_xmm1}, &regcls_sse, jive_resource_class_priority_reg_low,
	{}, &flt, 32, 128, 128);

const jive::register_class regcls_sse_xmm2(
	"sse_xmm2", {&reg_xmm2}, &regcls_sse, jive_resource_class_priority_reg_low,
	{}, &flt, 32, 128, 128);

const jive::register_class regcls_sse_xmm3(
	"sse_xmm3", {&reg_xmm3}, &regcls_sse, jive_resource_class_priority_reg_low,
	{}, &flt, 32, 128, 128);

const jive::register_class regcls_sse_xmm4(
	"sse_xmm4", {&reg_xmm4}, &regcls_sse, jive_resource_class_priority_reg_low,
	{}, &flt, 32, 128, 128);

const jive::register_class regcls_sse_xmm5(
	"sse_xmm5", {&reg_xmm5}, &regcls_sse, jive_resource_class_priority_reg_low,
	{}, &flt, 32, 128, 128);

const jive::register_class regcls_sse_xmm6(
	"sse_xmm6", {&reg_xmm6}, &regcls_sse, jive_resource_class_priority_reg_low,
	{}, &flt, 32, 128, 128);

const jive::register_class regcls_sse_xmm7(
	"sse_xmm7", {&reg_xmm7}, &regcls_sse, jive_resource_class_priority_reg_low,
	{}, &flt, 32, 128, 128);

}}
