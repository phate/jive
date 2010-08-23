#ifndef JIVE_BACKEND_I386_SUBROUTINE_H
#define JIVE_BACKEND_I386_SUBROUTINE_H

#include <jive/arch/subroutine.h>

struct jive_region;

jive_subroutine *
jive_i386_subroutine_create(
	struct jive_region * region,
	size_t narguments, const jive_argument_type arguments[],
	jive_argument_type return_type);

#endif
