#ifndef JIVE_BACKEND_I386_SUBROUTINE_H
#define JIVE_BACKEND_I386_SUBROUTINE_H

#include <jive/arch/subroutine.h>

struct jive_region;
struct jive_node;

typedef struct jive_i386_subroutine jive_i386_subroutine;

/* convert according to "default" ABI */
jive_subroutine *
jive_i386_subroutine_convert(struct jive_region * target_parent, struct jive_node * lambda_node);

jive_subroutine *
jive_i386_subroutine_create(struct jive_region * region,
	size_t nparameters, const jive_argument_type parameters[],
	size_t nreturns, const jive_argument_type returns[]);


struct jive_i386_subroutine {
	jive_subroutine base;
	
	jive_subroutine_passthrough saved_esp;
	jive_subroutine_passthrough saved_ebx;
	jive_subroutine_passthrough saved_ebp;
	jive_subroutine_passthrough saved_esi;
	jive_subroutine_passthrough saved_edi;
	
	jive_output * stackptr, * frameptr;
};

#endif
