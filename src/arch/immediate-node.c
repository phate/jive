/*
 * Copyright 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/immediate-node.h>

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include <jive/arch/immediate-type.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/nullary.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/simple_node.h>

/* immediate node */

namespace jive {

immediate_op::~immediate_op() noexcept {}

bool
immediate_op::operator==(const operation & other) const noexcept
{
	const immediate_op * op =
		dynamic_cast<const immediate_op *>(&other);
	return op && (op->value() == value());
}

const jive::base::type &
immediate_op::result_type(size_t index) const noexcept
{
	static const jive::imm::type type;
	return type;
}

const jive::port &
immediate_op::result(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	static const jive::imm::type type;
	static const jive::port p(type);
	return p;
}

std::string
immediate_op::debug_string() const
{
	return detail::strfmt(value().offset());
}

std::unique_ptr<jive::operation>
immediate_op::copy() const
{
	return std::unique_ptr<jive::operation>(new immediate_op(*this));
}

}

jive::output *
jive_immediate_create(
	jive::region * region,
	const jive::immediate * immediate_value)
{
	jive::immediate_op op(*immediate_value);
	return jive::create_normalized(region, op, {})[0];
}
