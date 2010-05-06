#ifndef JIVE_I386_ABI_H
#define JIVE_I386_ABI_H

#include <jive/types.h>
#include <jive/subroutine.h>

jive_subroutine *
jive_i386_subroutine_create(struct jive_graph * graph,
	const jive_argument_type arguments[],
	size_t narguments,
	jive_argument_type return_value);

#endif
