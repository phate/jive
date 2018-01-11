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

const jive::registers cc("cc", &regcls_flags, 0);
const jive::registers eax("eax", &regcls_gpr_eax, 0);
const jive::registers ecx("ecx", &regcls_gpr_ecx, 1);
const jive::registers edx("edx", &regcls_gpr_edx, 2);
const jive::registers ebx("ebx", &regcls_gpr_ebx, 3);
const jive::registers esi("esi", &regcls_gpr_esi, 6);
const jive::registers edi("edi", &regcls_gpr_edi, 7);
const jive::registers ebp("ebp", &regcls_gpr_ebp, 5);
const jive::registers esp("esp", &regcls_gpr_esp, 4);

const jive::registers st0("st0", &regcls_fp_st0, 0);

const jive::registers xmm0("xmm0", &regcls_sse_xmm0, 0);
const jive::registers xmm1("xmm1", &regcls_sse_xmm1, 1);
const jive::registers xmm2("xmm2", &regcls_sse_xmm2, 2);
const jive::registers xmm3("xmm3", &regcls_sse_xmm3, 3);
const jive::registers xmm4("xmm4", &regcls_sse_xmm4, 4);
const jive::registers xmm5("xmm5", &regcls_sse_xmm5, 5);
const jive::registers xmm6("xmm6", &regcls_sse_xmm6, 6);
const jive::registers xmm7("xmm7", &regcls_sse_xmm7, 7);

#define CLS(x) &regcls_##x
#define STACK4 &jive_stackslot_class_4_4

static const jive::bits::type bits16(16);
static const jive::bits::type bits32(32);
static const jive::flt::type flt;

const jive::register_class regcls_flags(
	"flags", {&cc}, &jive_root_register_class, jive_resource_class_priority_reg_high,
	{{CLS(gpr_eax), {CLS(flags), CLS(gpr_eax)}}, {STACK4, {CLS(flags), CLS(gpr_eax), STACK4}}},
	&bits16, 16, 0, 0);

const jive::register_class regcls_gpr(
	"gpr", {&eax, &ecx, &ebx, &edx, &esi, &edi, &ebp, &esp},
	&jive_root_register_class, jive_resource_class_priority_reg_low,
	{{STACK4, {CLS(gpr), STACK4}}}, &bits32, 32, 32, 8|16|32);

const jive::register_class regcls_gpr_byte(
	"gpr_byte_addressible", {&eax, &ecx, &ebx, &edx},
	&regcls_gpr, jive_resource_class_priority_reg_low,
	{{CLS(gpr), {CLS(gpr), CLS(gpr)}}, {STACK4, {CLS(gpr), STACK4}}},
	&bits32, 32, 32, 8|16|32);

const jive::register_class regcls_gpr_eax(
	"gpr_eax", {&eax}, &regcls_gpr_byte, jive_resource_class_priority_reg_low,
	{{CLS(gpr), {CLS(gpr), CLS(gpr)}}, {STACK4, {CLS(gpr), STACK4}}},
	&bits32, 32, 32, 8|16|32);

const jive::register_class regcls_gpr_ecx(
	"gpr_ecx", {&ecx}, &regcls_gpr_byte, jive_resource_class_priority_reg_low,
	{{CLS(gpr), {CLS(gpr), CLS(gpr)}}, {STACK4, {CLS(gpr), STACK4}}},
	&bits32, 32, 32, 8|16|32);

const jive::register_class regcls_gpr_ebx(
	"gpr_ebx", {&ebx}, &regcls_gpr_byte, jive_resource_class_priority_reg_low,
	{{CLS(gpr), {CLS(gpr), CLS(gpr)}}, {STACK4, {CLS(gpr), STACK4}}},
	&bits32, 32, 32, 8|16|32);

const jive::register_class regcls_gpr_edx(
	"gpr_edx", {&edx}, &regcls_gpr_byte, jive_resource_class_priority_reg_low,
	{{CLS(gpr), {CLS(gpr), CLS(gpr)}}, {STACK4, {CLS(gpr), STACK4}}},
	&bits32, 32, 32, 8|16|32);

const jive::register_class regcls_gpr_esi(
	"gpr_esi", {&esi}, &regcls_gpr, jive_resource_class_priority_reg_low,
	{{CLS(gpr), {CLS(gpr), CLS(gpr)}}, {STACK4, {CLS(gpr), STACK4}}},
	&bits32, 32, 32, 8|16|32);

const jive::register_class regcls_gpr_edi(
	"gpr_edi", {&edi}, &regcls_gpr, jive_resource_class_priority_reg_low,
	{{CLS(gpr), {CLS(gpr), CLS(gpr)}}, {STACK4, {CLS(gpr), STACK4}}},
	&bits32, 32, 32, 8|16|32);

const jive::register_class regcls_gpr_esp(
	"gpr_esp", {&esp}, &regcls_gpr, jive_resource_class_priority_reg_low,
	{{CLS(gpr), {CLS(gpr), CLS(gpr)}}, {STACK4, {CLS(gpr), STACK4}}},
	&bits32, 32, 32, 8|16|32);

const jive::register_class regcls_gpr_ebp(
	"gpr_ebp", {&ebp}, &regcls_gpr, jive_resource_class_priority_reg_low,
	{{CLS(gpr), {CLS(gpr), CLS(gpr)}}, {STACK4, {CLS(gpr), STACK4}}},
	&bits32, 32, 32, 8|16|32);

const jive::register_class regcls_fp(
	"fp", {&st0}, &jive_root_register_class, jive_resource_class_priority_reg_low,
	{{STACK4, {CLS(fp), STACK4}}}, &flt, 80, 80, 80);

const jive::register_class regcls_fp_st0(
	"fp_st0", {&st0}, &regcls_fp, jive_resource_class_priority_reg_low,
	{{CLS(fp), {CLS(fp), CLS(fp)}}, {STACK4, {CLS(gpr), STACK4}}},
	&flt, 80, 80, 80);

const jive::register_class regcls_sse(
	"sse", {&xmm0, &xmm1, &xmm2, &xmm3, &xmm4, &xmm5, &xmm6, &xmm7},
	&jive_root_register_class, jive_resource_class_priority_reg_low,
	{}, &flt, 32, 128, 128);

const jive::register_class regcls_sse_xmm0(
	"sse_xmm0", {&xmm0}, &regcls_sse, jive_resource_class_priority_reg_low,
	{}, &flt, 32, 128, 128);

const jive::register_class regcls_sse_xmm1(
	"sse_xmm1", {&xmm1}, &regcls_sse, jive_resource_class_priority_reg_low,
	{}, &flt, 32, 128, 128);

const jive::register_class regcls_sse_xmm2(
	"sse_xmm2", {&xmm2}, &regcls_sse, jive_resource_class_priority_reg_low,
	{}, &flt, 32, 128, 128);

const jive::register_class regcls_sse_xmm3(
	"sse_xmm3", {&xmm3}, &regcls_sse, jive_resource_class_priority_reg_low,
	{}, &flt, 32, 128, 128);

const jive::register_class regcls_sse_xmm4(
	"sse_xmm4", {&xmm4}, &regcls_sse, jive_resource_class_priority_reg_low,
	{}, &flt, 32, 128, 128);

const jive::register_class regcls_sse_xmm5(
	"sse_xmm5", {&xmm5}, &regcls_sse, jive_resource_class_priority_reg_low,
	{}, &flt, 32, 128, 128);

const jive::register_class regcls_sse_xmm6(
	"sse_xmm6", {&xmm6}, &regcls_sse, jive_resource_class_priority_reg_low,
	{}, &flt, 32, 128, 128);

const jive::register_class regcls_sse_xmm7(
	"sse_xmm7", {&xmm7}, &regcls_sse, jive_resource_class_priority_reg_low,
	{}, &flt, 32, 128, 128);

}}
