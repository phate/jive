/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

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

jive_subroutine *
jive_i386_subroutine_create_takeover(
	jive_context * context,
	size_t nparameters, jive_gate * const parameters[],
	size_t nreturns, jive_gate * const returns[],
	size_t npassthroughs, const jive_subroutine_passthrough passthroughs[]);

struct jive_i386_subroutine {
	jive_subroutine base;
};

extern const jive_subroutine_class JIVE_I386_SUBROUTINE;

#endif
