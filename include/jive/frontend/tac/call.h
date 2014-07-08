/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_TAC_CALL_H
#define JIVE_FRONTEND_TAC_CALL_H

#include <jive/frontend/tac/three_address_code.h>

class jive_basic_block;

namespace jive {
namespace frontend {

class call_code final : public three_address_code {
public:
	virtual ~call_code() noexcept;

	call_code(jive_basic_block * basic_block, struct jive_clg_node * callee,
		std::vector<three_address_code *> & arguments);

	virtual std::string debug_string() const override;

	struct jive_clg_node * callee;
};

}
}

jive::frontend::three_address_code *
jive_call_code_create(jive_basic_block * basic_block, struct jive_clg_node * callee,
	size_t narguments, jive::frontend::three_address_code * const arguments[]);

#endif
