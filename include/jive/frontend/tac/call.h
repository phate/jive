/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_TAC_CALL_H
#define JIVE_FRONTEND_TAC_CALL_H

#include <jive/frontend/tac/three_address_code.h>

class jive_call_code final : public jive_three_address_code {
public:
	virtual ~jive_call_code() noexcept;

	virtual std::string debug_string() const override;

	struct jive_clg_node * callee;
};

struct jive_three_address_code *
jive_call_code_create(struct jive_basic_block * basic_block, struct jive_clg_node * callee,
	size_t narguments, struct jive_three_address_code * const arguments[]);

#endif
