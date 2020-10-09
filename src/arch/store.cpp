/*
 * Copyright 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/store.hpp>

#include <jive/arch/address-transform.hpp>
#include <jive/arch/address.hpp>
#include <jive/arch/addresstype.hpp>
#include <jive/rvsdg/graph.hpp>
#include <jive/types/bitstring/type.hpp>

namespace jive {

/* store operator */

store_op::~store_op() noexcept
{}

bool
store_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const store_op *>(&other);
	return op
	    && op->addresstype() == addresstype()
	    && op->valuetype() == valuetype()
	    && op->nresults() == nresults();
}

std::vector<jive::port>
store_op::create_operands(
	const jive::valuetype & address,
	const jive::valuetype & value,
	size_t nstates)
{
	std::vector<jive::port> operands({address});
	operands.push_back(value);
	for (size_t n = 0; n < nstates; n++)
		operands.push_back({memtype()});

	return operands;
}

/* address store operator */

addrstore_op::~addrstore_op()
{}

std::string
addrstore_op::debug_string() const
{
	return "ADDRSTORE";
}

std::unique_ptr<operation>
addrstore_op::copy() const
{
	return std::unique_ptr<operation>(new addrstore_op(*this));
}

/* bitstring store operator */

bitstore_op::~bitstore_op()
{}

std::string
bitstore_op::debug_string() const
{
	return "BITSTORE";
}

std::unique_ptr<operation>
bitstore_op::copy() const
{
	return std::unique_ptr<operation>(new bitstore_op(*this));
}

}
