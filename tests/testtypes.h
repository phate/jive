/*
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TESTS_TESTTYPES_H
#define JIVE_TESTS_TESTTYPES_H

#include <jive/vsdg/statetype.h>
#include <jive/vsdg/valuetype.h>

/* test value type */

class jive_test_value_type final : public jive::value::type {
public:
	virtual ~jive_test_value_type() noexcept;

	inline constexpr jive_test_value_type() noexcept : jive::value::type() {};

	virtual std::string debug_string() const override;

	virtual bool operator==(const jive::base::type & other) const noexcept override;

	virtual jive_test_value_type * copy() const override;
};

/* test state type */

class jive_test_state_type final : public jive::state::type {
public:
	virtual ~jive_test_state_type() noexcept;

	inline constexpr jive_test_state_type() noexcept : jive::state::type() {};

	virtual std::string debug_string() const override;

	virtual bool operator==(const jive::base::type & other) const noexcept override;

	virtual jive_test_state_type * copy() const override;
};

#endif
