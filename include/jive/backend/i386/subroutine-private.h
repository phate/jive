#ifndef JIVE_BACKEND_I386_SUBROUTINE_PRIVATE_H
#define JIVE_BACKEND_I386_SUBROUTINE_PRIVATE_H

#include <jive/arch/subroutine-private.h>
#include <jive/arch/stackframe.h>
#include <jive/backend/i386/subroutine.h>

typedef struct jive_i386_subroutine jive_i386_subroutine;
typedef struct jive_i386_argument jive_i386_argument;

struct jive_i386_argument {
	jive_argument_type type;
	jive_stackslot * slot;
	jive_output * origin;
};

struct jive_i386_subroutine {
	jive_subroutine base;
	
	jive_stackframe * stackframe;
	size_t narguments;
	jive_i386_argument * arguments;
	jive_argument_type return_type;
	jive_gate * return_variable;
};

#endif
