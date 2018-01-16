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
	if (!op || op->narguments() != narguments() || op->nresults() != nresults())
		return false;

	for (size_t n = 0; n < narguments(); n++) {
		if (argument(n) != op->argument(n))
			return false;
	}

	for (size_t n = 0; n < nresults(); n++) {
		if (result(n) != op->result(n))
			return false;
	}

	return calling_convention() == op->calling_convention();
}

std::string
call_op::debug_string() const
{
	return "CALL";
}

std::vector<jive::port>
call_op::create_operands(
	const valuetype & address,
	const std::vector<const type*> & arguments)
{
	std::vector<jive::port> operands({address});
	for (const auto & type : arguments)
		operands.push_back({*type});

	return operands;
}

std::vector<jive::port>
call_op::create_results(const std::vector<const type*> & types)
{
	std::vector<port> results;
	for (const auto & type : types)
		results.push_back({*type});

	return results;
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
