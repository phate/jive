/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/compilate.h>
#include <jive/arch/instruction.h>
#include <jive/arch/label-mapper.h>
#include <jive/rvsdg/control.h>
#include <jive/rvsdg/graph.h>
#include <jive/rvsdg/region.h>
#include <jive/rvsdg/traverser.h>

#include <stdio.h>
#include <string.h>

namespace jive {

instruction_op::~instruction_op() noexcept
{
}

bool
instruction_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const instruction_op *>(&other);
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

	return op->icls() == icls();
}

std::string
instruction_op::debug_string() const
{
	return icls()->name();
}

std::unique_ptr<jive::operation>
instruction_op::copy() const
{
	return std::unique_ptr<jive::operation>(new instruction_op(*this));
}

std::vector<jive::port>
instruction_op::create_operands(
	const jive::instruction * icls,
	const std::vector<jive::port> & iports)
{
	static const immtype it;
	std::vector<jive::port> operands;
	for (size_t n = 0; n < icls->ninputs(); n++)
		operands.push_back(icls->input(n));
	for (size_t n = 0; n < icls->nimmediates(); n++)
		operands.push_back(it);

	operands.insert(operands.end(), iports.begin(), iports.end());
	return operands;
}

std::vector<jive::port>
instruction_op::create_results(
	const jive::instruction * icls,
	const std::vector<jive::port> & oports)
{
	std::vector<jive::port> results;
	for (size_t n = 0; n < icls->noutputs(); n++)
		results.push_back({icls->output(n)});

	results.insert(results.end(), oports.begin(), oports.end());
	return results;
}

}
