/*
 * Copyright 2010 2011 2012 2013 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_BACKEND_I386_REGISTERSET_H
#define JIVE_BACKEND_I386_REGISTERSET_H

#include <jive/arch/registers.h>

extern const jive::register_class jive_i386_regcls_gpr;
extern const jive::register_class jive_i386_regcls_fp;
extern const jive::register_class jive_i386_regcls_mmx;
extern const jive::register_class jive_i386_regcls_sse;
extern const jive::register_class jive_i386_regcls_flags;

/* gpr sub classes */
/* registers that are byte-addressible */
extern const jive::register_class jive_i386_regcls_gpr_byte;

extern const jive::register_class jive_i386_regcls_gpr_eax;
extern const jive::register_class jive_i386_regcls_gpr_ebx;
extern const jive::register_class jive_i386_regcls_gpr_ecx;
extern const jive::register_class jive_i386_regcls_gpr_edx;
extern const jive::register_class jive_i386_regcls_gpr_esi;
extern const jive::register_class jive_i386_regcls_gpr_edi;
extern const jive::register_class jive_i386_regcls_gpr_esp;
extern const jive::register_class jive_i386_regcls_gpr_ebp;

/* fp sub classes */
extern const jive::register_class jive_i386_regcls_fp_st0;

/* sse sub classes */
extern const jive::register_class jive_i386_regcls_sse_xmm0;
extern const jive::register_class jive_i386_regcls_sse_xmm1;
extern const jive::register_class jive_i386_regcls_sse_xmm2;
extern const jive::register_class jive_i386_regcls_sse_xmm3;
extern const jive::register_class jive_i386_regcls_sse_xmm4;
extern const jive::register_class jive_i386_regcls_sse_xmm5;
extern const jive::register_class jive_i386_regcls_sse_xmm6;
extern const jive::register_class jive_i386_regcls_sse_xmm7;

extern const jive::register_name jive_i386_reg_cc;
extern const jive::register_name jive_i386_reg_eax;
extern const jive::register_name jive_i386_reg_ecx;
extern const jive::register_name jive_i386_reg_edx;
extern const jive::register_name jive_i386_reg_ebx;
extern const jive::register_name jive_i386_reg_esi;
extern const jive::register_name jive_i386_reg_edi;
extern const jive::register_name jive_i386_reg_ebp;
extern const jive::register_name jive_i386_reg_esp;

extern const jive::register_name jive_i386_reg_st0;

extern const jive::register_name jive_i386_reg_xmm0;
extern const jive::register_name jive_i386_reg_xmm1;
extern const jive::register_name jive_i386_reg_xmm2;
extern const jive::register_name jive_i386_reg_xmm3;
extern const jive::register_name jive_i386_reg_xmm4;
extern const jive::register_name jive_i386_reg_xmm5;
extern const jive::register_name jive_i386_reg_xmm6;
extern const jive::register_name jive_i386_reg_xmm7;

#endif
