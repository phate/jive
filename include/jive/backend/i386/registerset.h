#ifndef JIVE_BACKEND_I386_REGISTERSET_H
#define JIVE_BACKEND_I386_REGISTERSET_H

#include <jive/arch/registers.h>

typedef enum {
	/* "top-level" register classes */
	jive_i386_gpr = 0,
	jive_i386_fp = 1,
	jive_i386_mmx = 2,
	jive_i386_sse = 3,
	jive_i386_flags = 8,
	
	/* sub classes */
	jive_i386_gpr_byte = 16, /* registers that are byte-addressible */
	jive_i386_gpr_eax = 17,	
	jive_i386_gpr_ebx = 18,
	jive_i386_gpr_ecx = 19,	
	jive_i386_gpr_edx = 20,
	jive_i386_gpr_esi = 21,
	jive_i386_gpr_edi = 22,
	jive_i386_gpr_esp = 23,
	jive_i386_gpr_ebp = 24,
} jive_i386_regcls_index;

extern const struct jive_register_class jive_i386_regcls[];

typedef enum {
	jive_i386_cc = 0,
	jive_i386_eax = 1,
	jive_i386_ecx = 2,
	jive_i386_ebx = 3,
	jive_i386_edx = 4,
	jive_i386_esi = 5,
	jive_i386_edi = 6,
	jive_i386_ebp = 7,
	jive_i386_esp = 8,
} jive_i386_reg_index;

extern const jive_register_name jive_i386_regs [];

#endif
