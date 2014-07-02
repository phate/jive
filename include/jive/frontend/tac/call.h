/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_TAC_CALL_H
#define JIVE_FRONTEND_TAC_CALL_H

#include <jive/frontend/tac/three_address_code.h>

extern const jive_three_address_code_class JIVE_CALL_CODE;

typedef struct jive_call_code_attrs jive_call_code_attrs;

struct jive_call_code_attrs {
	jive_three_address_code_attrs base;
	struct jive_clg_node * callee;
};

class jive_call_code final : public jive_three_address_code {
public:
	virtual ~jive_call_code() noexcept;

	jive_call_code_attrs attrs;
};

static inline jive_call_code *
jive_call_code_cast(struct jive_three_address_code * tac)
{
	if (jive_three_address_code_isinstance(tac, &JIVE_CALL_CODE))
		return (jive_call_code *) tac;
	else
		return 0;
}

struct jive_three_address_code *
jive_call_code_create(struct jive_basic_block * basic_block, struct jive_clg_node * callee,
	size_t narguments, struct jive_three_address_code * const arguments[]);

#endif
