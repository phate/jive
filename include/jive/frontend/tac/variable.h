/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_TAC_VARIABLE_H
#define JIVE_FRONTEND_TAC_VARIABLE_H

#include <jive/frontend/tac/three_address_code.h>

extern const jive_three_address_code_class JIVE_VARIABLE_CODE;

typedef struct jive_variable_code jive_variable_code;
typedef struct jive_variable_code_attrs jive_variable_code_attrs;

struct jive_variable_code_attrs {
	jive_three_address_code_attrs base;
	char * name;
};

struct jive_variable_code {
	jive_three_address_code base;
	jive_variable_code_attrs attrs;
};

static inline struct jive_variable_code *
jive_variable_code_cast(const struct jive_three_address_code * tac)
{
	if (jive_three_address_code_isinstance(tac, &JIVE_VARIABLE_CODE))
		return (jive_variable_code *) tac;
	else
		return 0;
}

static inline const struct jive_variable_code *
jive_variable_code_const_cast(const struct jive_three_address_code * tac)
{
	if (jive_three_address_code_isinstance(tac, &JIVE_VARIABLE_CODE))
		return (const jive_variable_code *) tac;
	else
		return 0;
}

struct jive_three_address_code *
jive_variable_code_create(struct jive_basic_block * basic_block, const char * name);

#endif