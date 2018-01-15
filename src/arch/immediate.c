/*
 * Copyright 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/immediate.h>

namespace jive {

/* immediate type */

immtype::~immtype() noexcept
{}

std::string
immtype::debug_string() const
{
	return "imm";
}

bool
immtype::operator==(const jive::type & other) const noexcept
{
	return dynamic_cast<const immtype*>(&other) != nullptr;
}

std::unique_ptr<jive::type>
immtype::copy() const
{
	return std::unique_ptr<jive::type>(new immtype(*this));
}

/* immediate operator */

immediate_op::~immediate_op() noexcept
{}

bool
immediate_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const immediate_op*>(&other);
	return op && (op->value() == value());
}

const jive::port &
immediate_op::result(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	static const immtype it;
	static const jive::port p(it);
	return p;
}

std::string
immediate_op::debug_string() const
{
	return detail::strfmt("IMM(", value().offset(), ")");
}

std::unique_ptr<jive::operation>
immediate_op::copy() const
{
	return std::unique_ptr<jive::operation>(new immediate_op(*this));
}

}
