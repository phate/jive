/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/frontend/basic_block.h>
#include <jive/frontend/cfg.h>
#include <jive/frontend/tac/call.h>
#include <jive/frontend/tac/three_address_code-private.h>
#include <jive/frontend/clg.h>
#include <jive/frontend/clg_node.h>
#include <jive/util/buffer.h>

#include <stdio.h>

jive_call_code::~jive_call_code() noexcept {}

std::string
jive_call_code::debug_string() const
{
	std::string label("call ");
	label.append(attrs.callee->name);
	label.append("(");

	size_t n;
	char tmp[32];
	for (n = 0; n < operands.size(); n++) {
		snprintf(tmp, sizeof(tmp), "%p", operands[n]);
		label.append(tmp);
		if (n != operands.size()-1)
			label.append(", ");
	}

	label.append("9");
	return label;
}

static const struct jive_three_address_code_attrs *
jive_call_code_get_attrs_(const struct jive_three_address_code * self);

const struct jive_three_address_code_class JIVE_CALL_CODE = {
	parent : &JIVE_CALL_CODE,
	name : "CALL",
	fini : nullptr, /* inherit */
	get_attrs : jive_call_code_get_attrs_, /* override */
};

static void
jive_call_code_init_(jive_call_code * self, struct jive_basic_block * basic_block,
	struct jive_clg_node * callee,
	size_t narguments, struct jive_three_address_code * const arguments[])
{
	jive_three_address_code_init_(self, basic_block, narguments, arguments);
	self->attrs.callee = callee;
	jive_clg_node_add_call(basic_block->base.cfg->clg_node, callee);
}

static const struct jive_three_address_code_attrs *
jive_call_code_get_attrs_(const struct jive_three_address_code * self_)
{
	const struct jive_call_code_attrs * self = (const struct jive_call_code_attrs *) self_;
	return &self->base;
}

jive_three_address_code *
jive_call_code_create(struct jive_basic_block * basic_block, jive_clg_node * callee,
	size_t narguments, struct jive_three_address_code * const arguments[])
{
	jive_call_code * call = new jive_call_code;
	call->class_ = &JIVE_CALL_CODE;
	jive_call_code_init_(call, basic_block, callee, narguments, arguments);
	return call;
}
