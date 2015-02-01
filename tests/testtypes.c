/*
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "testtypes.h"

#include <jive/util/buffer.h>

/* test value type */

jive_test_value_type::~jive_test_value_type() noexcept {}

std::string
jive_test_value_type::debug_string() const
{
	return "test_value";
}

bool
jive_test_value_type::operator==(const jive::base::type & other) const noexcept
{
	return dynamic_cast<const jive_test_value_type*>(&other) != nullptr;
}

jive_test_value_type *
jive_test_value_type::copy() const
{
	return new jive_test_value_type();
}

/* test state type */

jive_test_state_type::~jive_test_state_type() noexcept {}

std::string
jive_test_state_type::debug_string() const
{
	return "test_state";
}

bool
jive_test_state_type::operator==(const jive::base::type & other) const noexcept
{
	return dynamic_cast<const jive_test_state_type*>(&other) != nullptr;
}

jive_test_state_type *
jive_test_state_type::copy() const
{
	return new jive_test_state_type();
}
