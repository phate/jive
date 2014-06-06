/*
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
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

	virtual void label(jive_buffer & buffer) const override;

	virtual bool operator==(const jive::base::type & other) const noexcept override;

	virtual jive_test_value_type * copy() const override;

	virtual jive_input * create_input(jive_node * node, size_t index,
		jive_output * origin) const override;

	virtual jive_output * create_output(jive_node * node, size_t index) const override;

	virtual jive_gate * create_gate(jive_graph * graph, const char * name) const override;
};

class jive_test_value_input final : public jive::value::input {
public:
	virtual ~jive_test_value_input() noexcept;

	jive_test_value_input(jive_node * node, size_t index, jive_output * output);

	virtual const jive_test_value_type & type() const noexcept { return type_; }

private:
	jive_test_value_type type_;
};

class jive_test_value_output final : public jive::value::output {
public:
	virtual ~jive_test_value_output() noexcept;

	jive_test_value_output(jive_node * node, size_t index);

	virtual const jive_test_value_type & type() const noexcept { return type_; }

private:
	jive_test_value_type type_;
};

class jive_test_value_gate final : public jive::value::gate {
public:
	virtual ~jive_test_value_gate() noexcept;

	jive_test_value_gate(jive_graph * graph, const char name[]);

	virtual const jive_test_value_type & type() const noexcept { return type_; }

private:
	jive_test_value_type type_;
};

/* test state type */

class jive_test_state_type final : public jive::state::type {
public:
	virtual ~jive_test_state_type() noexcept;

	inline constexpr jive_test_state_type() noexcept : jive::state::type() {};

	virtual void label(jive_buffer & buffer) const override;

	virtual bool operator==(const jive::base::type & other) const noexcept override;

	virtual jive_test_state_type * copy() const override;

	virtual jive_input * create_input(jive_node * node, size_t index,
		jive_output * origin) const override;

	virtual jive_output * create_output(jive_node * node, size_t index) const override;

	virtual jive_gate * create_gate(jive_graph * graph, const char * name) const override;
};

class jive_test_state_input final : public jive::state::input {
public:
	virtual ~jive_test_state_input() noexcept;

	jive_test_state_input(jive_node * node, size_t index, jive_output * origin);

	virtual const jive_test_state_type & type() const noexcept { return type_; }

private:
	jive_test_state_type type_;
};

class jive_test_state_output final : public jive::state::output {
public:
	virtual ~jive_test_state_output() noexcept;

	jive_test_state_output(jive_node * node, size_t index);

	virtual const jive_test_state_type & type() const noexcept { return type_; }

private:
	jive_test_state_type type_;
};

class jive_test_state_gate final : public jive::state::gate {
public:
	virtual ~jive_test_state_gate() noexcept;

	jive_test_state_gate(jive_graph * graph, const char name[]);

	virtual const jive_test_state_type & type() const noexcept { return type_; }

private:
	jive_test_state_type type_;
};

#endif
