/*
 * Copyright 2010 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "testnodes.h"

#include <jive/util/ptr-collection.h>

namespace jive {
namespace test {

/* unary operation */

unary_op::~unary_op() noexcept
{}

jive_unop_reduction_path_t
unary_op::can_reduce_operand(const jive::output * operand) const noexcept
{
	return jive_unop_reduction_none;
}

jive::output *
unary_op::reduce_operand(
	jive_unop_reduction_path_t path,
	jive::output * operand) const
{
	return nullptr;
}

std::string
unary_op::debug_string() const
{
	return "UNARY_TEST_NODE";
}

std::unique_ptr<jive::operation>
unary_op::copy() const
{
	return std::unique_ptr<jive::operation>(new unary_op(*this));
}

/* simple operation */

simple_op::~simple_op() noexcept {}

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
