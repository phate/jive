/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/call.h>

namespace jive {

/* call operation */

call_op::~call_op() noexcept
{}

bool
call_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const call_op*>(&other);
	return op
	    && op->address_ == address_
	    && op->calling_convention() == calling_convention()
	    && op->arguments_ == arguments_
	    && op->results_ == results_;
}

size_t
call_op::narguments() const noexcept
{
	return 1 + arguments_.size();
}

const jive::port &
call_op::argument(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < narguments());

	if (index == 0)
		return address_;

	return arguments_[index-1];
}

size_t
call_op::nresults() const noexcept
{
	return results_.size();
}

const jive::port &
call_op::result(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	return results_[index];
}

std::string
call_op::debug_string() const
{
	return "CALL";
}

/* address call operation */

addrcall_op::~addrcall_op()
{}

std::unique_ptr<operation>
addrcall_op::copy() const
{
	return std::unique_ptr<operation>(new addrcall_op(*this));
}

/* bitstring call operation */

bitcall_op::~bitcall_op()
{}

std::unique_ptr<operation>
bitcall_op::copy() const
{
	return std::unique_ptr<operation>(new bitcall_op(*this));
}

}
