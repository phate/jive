/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_BACKEND_I386_SUBROUTINE_H
#define JIVE_BACKEND_I386_SUBROUTINE_H

#include <jive/arch/subroutine.h>

struct jive_node;

jive_subroutine
jive_i386_subroutine_begin(jive_graph * graph,
	size_t nparameters, const jive_argument_type parameter_types[],
	size_t nreturns, const jive_argument_type return_types[]);

/* convert according to "default" ABI */
jive_node *
jive_i386_subroutine_convert(jive::region * target_parent, jive_node * lambda_node);

#endif
