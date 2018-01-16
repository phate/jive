/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/rvsdg/graph.h>
#include <jive/types/function/fctapply.h>
#include <jive/types/function/fcttype.h>

#include <string.h>

namespace jive {
namespace fct {

apply_op::~apply_op() noexcept
{
}

bool
apply_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const apply_op*>(&other);
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

	return true;
}

std::string
apply_op::debug_string() const
{
	return "APPLY";
}

std::unique_ptr<jive::operation>
apply_op::copy() const
{
	return std::unique_ptr<jive::operation>(new apply_op(*this));
}

std::vector<jive::port>
apply_op::create_operands(const fct::type & type)
{
	std::vector<jive::port> operands({type});
	for (size_t n = 0; n < type.narguments(); n++)
		operands.push_back(type.argument_type(n));

	return operands;
}

std::vector<jive::port>
apply_op::create_results(const fct::type & type)
{
	std::vector<jive::port> results;
	for (size_t n = 0; n < type.nresults(); n++)
		results.push_back({type.result_type(n)});

	return results;
}

}
}
