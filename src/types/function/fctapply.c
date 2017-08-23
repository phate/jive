/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/function/fctapply.h>

#include <jive/types/function/fcttype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/simple_node.h>

#include <string.h>

namespace jive {
namespace fct {

apply_op::~apply_op() noexcept
{
}

apply_op::apply_op(const type & fcttype)
: simple_op()
{
	arguments_.push_back({fcttype});
	for (size_t n = 0; n < fcttype.narguments(); n++)
		arguments_.push_back({fcttype.argument_type(n)});

	for (size_t n = 0; n < fcttype.nresults(); n++)
		results_.push_back({fcttype.result_type(n)});
}

bool
apply_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const apply_op*>(&other);
	return op && op->results_ == results_ && op->arguments_ == arguments_;
}

size_t
apply_op::narguments() const noexcept
{
	return arguments_.size();
}

const jive::port &
apply_op::argument(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < narguments());
	return arguments_[index];
}

size_t
apply_op::nresults() const noexcept
{
	return results_.size();
}

const jive::base::type &
apply_op::result_type(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	return results_[index].type();
}

const jive::port &
apply_op::result(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	return results_[index];
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

std::vector<jive::output*>
jive_apply_create(jive::output * function, size_t narguments, jive::output * const arguments[])
{
	jive::fct::apply_op op(dynamic_cast<const jive::fct::type &>(function->type()));
	std::vector<jive::output*> apply_args;
	apply_args.push_back(function);
	for (size_t n = 0; n < narguments; ++n) {
		apply_args.push_back(arguments[n]);
	}

	jive::region * region = apply_args[0]->region();
	return jive::create_normalized(region, op, apply_args);
}
