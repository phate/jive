#ifndef JIVE_I386_ABI_H
#define JIVE_I386_ABI_H

#include <jive/types.h>

jive_subroutine *
jive_i386_subroutine_create(jive_graph * graph,
	size_t nparams, bool return_value);

#endif
