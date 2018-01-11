/*
 * Copyright 2010 2011 2012 2013 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_BACKEND_I386_REGISTERSET_H
#define JIVE_BACKEND_I386_REGISTERSET_H

#include <jive/arch/registers.h>

namespace jive {
namespace i386 {

/* registers */

extern const jive::registers cc;
extern const jive::registers st0;
extern const jive::registers eax, ebx, ecx, edx, edi, esi, ebp, esp;
extern const jive::registers xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;

/* register classes */

extern const jive::register_class gpr_regcls;
extern const jive::register_class fp_regcls;
extern const jive::register_class mmx_regcls;
extern const jive::register_class xmm_regcls;
extern const jive::register_class cc_regcls;

/* gpr sub classes */
/* registers that are byte-addressible */
extern const jive::register_class gprbyte_regcls;

extern const jive::register_class eax_regcls;
extern const jive::register_class ebx_regcls;
extern const jive::register_class ecx_regcls;
extern const jive::register_class edx_regcls;
extern const jive::register_class esi_regcls;
extern const jive::register_class edi_regcls;
extern const jive::register_class esp_regcls;
extern const jive::register_class ebp_regcls;

/* fp sub classes */
extern const jive::register_class st0_regcls;

/* sse sub classes */
extern const jive::register_class xmm0_regcls;
extern const jive::register_class xmm1_regcls;
extern const jive::register_class xmm2_regcls;
extern const jive::register_class xmm3_regcls;
extern const jive::register_class xmm4_regcls;
extern const jive::register_class xmm5_regcls;
extern const jive::register_class xmm6_regcls;
extern const jive::register_class xmm7_regcls;

}}

#endif
