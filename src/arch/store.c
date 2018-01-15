/*
 * Copyright 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/store.h>

#include <jive/arch/address-transform.h>
#include <jive/arch/address.h>
#include <jive/arch/addresstype.h>
#include <jive/rvsdg/graph.h>
#include <jive/types/bitstring/type.h>

namespace jive {

/* store operator */

store_op::~store_op() noexcept
{}

bool
store_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const store_op *>(&other);
	return op
	    && op->address_ == address_
	    && op->value_ == value_
	    && op->nstates_ == nstates_;
}

size_t
store_op::narguments() const noexcept
{
	return 2 + nstates_;
}

const jive::port &
store_op::argument(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < narguments());

	if (index == 0)
		return address_;

	if (index == 1)
		return value_;

	static const jive::memtype mt;
	static const jive::port p(mt);
	return p;
}

size_t
store_op::nresults() const noexcept
{
	return nstates_;
}

const jive::port &
store_op::result(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());

	static const jive::memtype mt;
	static const jive::port p(mt);
	return p;
}

std::string
store_op::debug_string() const
{
	return "STORE";
}

std::unique_ptr<jive::operation>
store_op::copy() const
{
	return std::unique_ptr<jive::operation>(new store_op(*this));
}

/* address store operator */

addrstore_op::~addrstore_op()
{}

/* bitstring store operator */

bitstore_op::~bitstore_op()
{}

}
