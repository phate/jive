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

	virtual jive::gate * create_gate(jive_graph * graph, const char * name) const override;
};

class jive_test_value_gate final : public jive::value::gate {
public:
	virtual ~jive_test_value_gate() noexcept;

	jive_test_value_gate(jive_graph * graph, const char name[]);

private:
	jive_test_value_gate(const jive_test_value_gate & rhs) = delete;
	jive_test_value_gate& operator=(const jive_test_value_gate & rhs) = delete;

	jive_test_value_type type_;
};

/* test state type */

class jive_test_state_type final : public jive::state::type {
public:
	virtual ~jive_test_state_type() noexcept;

	inline constexpr jive_test_state_type() noexcept : jive::state::type() {};

	virtual std::string debug_string() const override;

	virtual bool operator==(const jive::base::type & other) const noexcept override;

	virtual jive_test_state_type * copy() const override;

	virtual jive::gate * create_gate(jive_graph * graph, const char * name) const override;
};

class jive_test_state_gate final : public jive::state::gate {
public:
	virtual ~jive_test_state_gate() noexcept;

	jive_test_state_gate(jive_graph * graph, const char name[]);

private:
	jive_test_state_gate(const jive_test_state_gate & rhs) = delete;
	jive_test_state_gate& operator=(const jive_test_state_gate & rhs) = delete;

	jive_test_state_type type_;
};

#endif
