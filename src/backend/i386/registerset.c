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

const jive::registers cc("cc", &cc_regcls, 0);
const jive::registers eax("eax", &eax_regcls, 0);
const jive::registers ecx("ecx", &ecx_regcls, 1);
const jive::registers edx("edx", &edx_regcls, 2);
const jive::registers ebx("ebx", &ebx_regcls, 3);
const jive::registers esi("esi", &esi_regcls, 6);
const jive::registers edi("edi", &edi_regcls, 7);
const jive::registers ebp("ebp", &ebp_regcls, 5);
const jive::registers esp("esp", &esp_regcls, 4);

const jive::registers st0("st0", &st0_regcls, 0);

const jive::registers xmm0("xmm0", &xmm0_regcls, 0);
const jive::registers xmm1("xmm1", &xmm1_regcls, 1);
const jive::registers xmm2("xmm2", &xmm2_regcls, 2);
const jive::registers xmm3("xmm3", &xmm3_regcls, 3);
const jive::registers xmm4("xmm4", &xmm4_regcls, 4);
const jive::registers xmm5("xmm5", &xmm5_regcls, 5);
const jive::registers xmm6("xmm6", &xmm6_regcls, 6);
const jive::registers xmm7("xmm7", &xmm7_regcls, 7);

#define CLS(x) &x##_regcls
#define STACK4 &jive_stackslot_class_4_4

static const jive::bits::type bits16(16);
static const jive::bits::type bits32(32);
static const jive::flt::type flt;

const jive::register_class cc_regcls(
	"cc", {&cc}, &jive_root_register_class, jive_resource_class_priority_reg_high,
	{{CLS(eax), {CLS(cc), CLS(eax)}}, {STACK4, {CLS(cc), CLS(eax), STACK4}}},
	&bits16, 16, 0, 0);

const jive::register_class gpr_regcls(
	"gpr", {&eax, &ecx, &ebx, &edx, &esi, &edi, &ebp, &esp},
	&jive_root_register_class, jive_resource_class_priority_reg_low,
	{{STACK4, {CLS(gpr), STACK4}}}, &bits32, 32, 32, 8|16|32);

const jive::register_class gprbyte_regcls(
	"gprbyte", {&eax, &ecx, &ebx, &edx},
	&gpr_regcls, jive_resource_class_priority_reg_low,
	{{CLS(gpr), {CLS(gpr), CLS(gpr)}}, {STACK4, {CLS(gpr), STACK4}}},
	&bits32, 32, 32, 8|16|32);

const jive::register_class eax_regcls(
	"eax", {&eax}, &gprbyte_regcls, jive_resource_class_priority_reg_low,
	{{CLS(gpr), {CLS(gpr), CLS(gpr)}}, {STACK4, {CLS(gpr), STACK4}}},
	&bits32, 32, 32, 8|16|32);

const jive::register_class ecx_regcls(
	"ecx", {&ecx}, &gprbyte_regcls, jive_resource_class_priority_reg_low,
	{{CLS(gpr), {CLS(gpr), CLS(gpr)}}, {STACK4, {CLS(gpr), STACK4}}},
	&bits32, 32, 32, 8|16|32);

const jive::register_class ebx_regcls(
	"ebx", {&ebx}, &gprbyte_regcls, jive_resource_class_priority_reg_low,
	{{CLS(gpr), {CLS(gpr), CLS(gpr)}}, {STACK4, {CLS(gpr), STACK4}}},
	&bits32, 32, 32, 8|16|32);

const jive::register_class edx_regcls(
	"edx", {&edx}, &gprbyte_regcls, jive_resource_class_priority_reg_low,
	{{CLS(gpr), {CLS(gpr), CLS(gpr)}}, {STACK4, {CLS(gpr), STACK4}}},
	&bits32, 32, 32, 8|16|32);

const jive::register_class esi_regcls(
	"esi", {&esi}, &gpr_regcls, jive_resource_class_priority_reg_low,
	{{CLS(gpr), {CLS(gpr), CLS(gpr)}}, {STACK4, {CLS(gpr), STACK4}}},
	&bits32, 32, 32, 8|16|32);

const jive::register_class edi_regcls(
	"edi", {&edi}, &gpr_regcls, jive_resource_class_priority_reg_low,
	{{CLS(gpr), {CLS(gpr), CLS(gpr)}}, {STACK4, {CLS(gpr), STACK4}}},
	&bits32, 32, 32, 8|16|32);

const jive::register_class esp_regcls(
	"esp", {&esp}, &gpr_regcls, jive_resource_class_priority_reg_low,
	{{CLS(gpr), {CLS(gpr), CLS(gpr)}}, {STACK4, {CLS(gpr), STACK4}}},
	&bits32, 32, 32, 8|16|32);

const jive::register_class ebp_regcls(
	"ebp", {&ebp}, &gpr_regcls, jive_resource_class_priority_reg_low,
	{{CLS(gpr), {CLS(gpr), CLS(gpr)}}, {STACK4, {CLS(gpr), STACK4}}},
	&bits32, 32, 32, 8|16|32);

const jive::register_class fp_regcls(
	"fp", {&st0}, &jive_root_register_class, jive_resource_class_priority_reg_low,
	{{STACK4, {CLS(fp), STACK4}}}, &flt, 80, 80, 80);

const jive::register_class st0_regcls(
	"st0", {&st0}, &fp_regcls, jive_resource_class_priority_reg_low,
	{{CLS(fp), {CLS(fp), CLS(fp)}}, {STACK4, {CLS(gpr), STACK4}}},
	&flt, 80, 80, 80);

const jive::register_class xmm_regcls(
	"xmm", {&xmm0, &xmm1, &xmm2, &xmm3, &xmm4, &xmm5, &xmm6, &xmm7},
	&jive_root_register_class, jive_resource_class_priority_reg_low,
	{}, &flt, 32, 128, 128);

const jive::register_class xmm0_regcls(
	"xmm0", {&xmm0}, &xmm_regcls, jive_resource_class_priority_reg_low,
	{}, &flt, 32, 128, 128);

const jive::register_class xmm1_regcls(
	"xmm1", {&xmm1}, &xmm_regcls, jive_resource_class_priority_reg_low,
	{}, &flt, 32, 128, 128);

const jive::register_class xmm2_regcls(
	"xmm2", {&xmm2}, &xmm_regcls, jive_resource_class_priority_reg_low,
	{}, &flt, 32, 128, 128);

const jive::register_class xmm3_regcls(
	"xmm3", {&xmm3}, &xmm_regcls, jive_resource_class_priority_reg_low,
	{}, &flt, 32, 128, 128);

const jive::register_class xmm4_regcls(
	"xmm4", {&xmm4}, &xmm_regcls, jive_resource_class_priority_reg_low,
	{}, &flt, 32, 128, 128);

const jive::register_class xmm5_regcls(
	"xmm5", {&xmm5}, &xmm_regcls, jive_resource_class_priority_reg_low,
	{}, &flt, 32, 128, 128);

const jive::register_class xmm6_regcls(
	"xmm6", {&xmm6}, &xmm_regcls, jive_resource_class_priority_reg_low,
	{}, &flt, 32, 128, 128);

const jive::register_class xmm7_regcls(
	"xmm7", {&xmm7}, &xmm_regcls, jive_resource_class_priority_reg_low,
	{}, &flt, 32, 128, 128);

}}
