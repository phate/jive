/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/function/fctapply.h>

#include <jive/types/function/fcttype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>

#include <string.h>

namespace jive {
namespace fct {

apply_op::~apply_op() noexcept
{
}

apply_op::apply_op(const type & function_type)
	: function_type_(function_type)
{
}

bool
apply_op::operator==(const operation & other) const noexcept
{
	const apply_op * op =
		dynamic_cast<const apply_op *>(&other);
	return op && op->function_type() == function_type();
}

size_t
apply_op::narguments() const noexcept
{
	return function_type().narguments() + 1;
}

const jive::base::type &
apply_op::argument_type(size_t index) const noexcept
{
	if (index == 0) {
		return function_type();
	} else {
		return *function_type().argument_type(index - 1);
	}
}

size_t
apply_op::nresults() const noexcept
{
	return function_type().nreturns();
}

const jive::base::type &
apply_op::result_type(size_t index) const noexcept
{
	return *function_type().return_type(index);
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

}
}

std::vector<jive::output *>
jive_apply_create(jive::output * function, size_t narguments, jive::output * const arguments[])
{
	jive::fct::apply_op op(dynamic_cast<const jive::fct::type &>(function->type()));
	std::vector<jive::oport*> apply_args;
	apply_args.push_back(function);
	for (size_t n = 0; n < narguments; ++n) {
		apply_args.push_back(arguments[n]);
	}

	jive_region * region = apply_args[0]->region();
	return jive_node_create_normalized(region, op, apply_args);
}
