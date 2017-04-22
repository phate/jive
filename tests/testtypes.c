/*
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "testtypes.h"

namespace jive {
namespace test {

/* test value type */

valuetype::~valuetype() noexcept
{}

std::string
valuetype::debug_string() const
{
	return "test_value";
}

bool
valuetype::operator==(const jive::base::type & other) const noexcept
{
	return dynamic_cast<const valuetype*>(&other) != nullptr;
}

std::unique_ptr<base::type>
valuetype::copy() const
{
	return std::unique_ptr<base::type>(new valuetype(*this));
}

/* test state type */

statetype::~statetype() noexcept
{}

std::string
statetype::debug_string() const
{
	return "test_state";
}

bool
statetype::operator==(const jive::base::type & other) const noexcept
{
	return dynamic_cast<const statetype*>(&other) != nullptr;
}

std::unique_ptr<base::type>
statetype::copy() const
{
	return std::unique_ptr<base::type>(new statetype(*this));
}

}}
