#ifndef JIVE_BACKEND_I386_REGISTERSET_H
#define JIVE_BACKEND_I386_REGISTERSET_H

#include <jive/arch/registers.h>

typedef enum {
	jive_i386_flags = 0,
	jive_i386_gpr = 1,
	jive_i386_gpr_byte = 2,	/* registers that are byte-addressible */
	jive_i386_gpr_eax = 3,	
	jive_i386_gpr_ebx = 4,
	jive_i386_gpr_ecx = 5,	
	jive_i386_gpr_edx = 6,
	jive_i386_gpr_esi = 7,
	jive_i386_gpr_edi = 8,
	jive_i386_gpr_esp = 9,
	jive_i386_gpr_ebp = 10,
} jive_i386_regcls_index;

extern const struct jive_regcls jive_i386_regcls[];

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

extern const jive_cpureg jive_i386_regs [];

#endif
