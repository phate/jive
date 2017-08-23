/*
 * Copyright 2010 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "testnodes.h"

#include <jive/util/ptr-collection.h>

namespace jive {
namespace test {

/* simple operation */

simple_op::~simple_op() noexcept {}

simple_op::simple_op(
	const std::vector<const jive::base::type*> & argument_types,
	const std::vector<const jive::base::type*> & result_types)
{
	for (const auto & type : argument_types)
		arguments_.push_back({*type->copy()});
	for (const auto & type : result_types)
		results_.push_back({*type->copy()});
}

bool
simple_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const jive::test::simple_op*>(&other);
	return op
	    && arguments_ == op->arguments_
	    && results_ == op->results_;
}

size_t
simple_op::narguments() const noexcept
{
	return arguments_.size();
}

const jive::port &
simple_op::argument(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < narguments());
	return arguments_[index];
}

size_t
simple_op::nresults() const noexcept
{
	return results_.size();
}

const jive::base::type &
simple_op::result_type(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	return results_[index].type();
}

const jive::port &
simple_op::result(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	return results_[index];
}

std::string
simple_op::debug_string() const
{
	return "SIMPLE_TEST_NODE";
}

std::unique_ptr<jive::operation>
simple_op::copy() const
{
	return std::unique_ptr<jive::operation>(new jive::test::simple_op(*this));
}

/* structural operation */

structural_op::~structural_op()
{}

std::string
structural_op::debug_string() const
{
	return "STRUCTURAL_TEST_NODE";
}

std::unique_ptr<jive::operation>
structural_op::copy() const
{
	return std::unique_ptr<jive::operation>(new structural_op(*this));
}

}}
