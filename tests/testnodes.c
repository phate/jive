/*
 * Copyright 2010 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "testnodes.h"

#include <jive/util/ptr-collection.h>

namespace jive {
namespace test {

op::~op() noexcept {}

op::op(
	const std::vector<const jive::base::type*> & argument_types,
	const std::vector<const jive::base::type*> & result_types)
	: argument_types_(jive::detail::unique_ptr_vector_copy(argument_types))
	, result_types_(jive::detail::unique_ptr_vector_copy(result_types))
{}

op::op(const jive::test::op & other)
	: argument_types_(jive::detail::unique_ptr_vector_copy(other.argument_types_))
	, result_types_(jive::detail::unique_ptr_vector_copy(other.result_types_))
{}

bool
op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const jive::test::op*>(&other);
	return op &&
		jive::detail::ptr_container_equals(argument_types_, op->argument_types_) &&
		jive::detail::ptr_container_equals(result_types_, op->result_types_);
}

size_t
op::narguments() const noexcept
{
	return argument_types_.size();
}

const jive::base::type &
op::argument_type(size_t index) const noexcept
{
	return *argument_types_[index];
}

size_t
op::nresults() const noexcept
{
	return result_types_.size();
}

const jive::base::type &
op::result_type(size_t index) const noexcept
{
	return *result_types_[index];
}

std::string
op::debug_string() const
{
	return "TEST_NODE";
}

std::unique_ptr<jive::operation>
op::copy() const
{
	return std::unique_ptr<jive::operation>(new jive::test::op(*this));
}

}}
