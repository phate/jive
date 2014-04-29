/*
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TESTS_TESTTYPES_H
#define JIVE_TESTS_TESTTYPES_H

#include <jive/vsdg/statetype.h>
#include <jive/vsdg/valuetype.h>

/* test value type */

typedef struct jive_test_value_type jive_test_value_type;

extern const jive_type_class JIVE_TEST_VALUE_TYPE;
#define JIVE_DECLARE_TEST_VALUE_TYPE(name) \
	jive_test_value_type name##_struct; name##_struct.class_ = &JIVE_TEST_VALUE_TYPE; \
	const jive_type * name = &name##_struct

class jive_test_value_type final : public jive_value_type {
public:
	virtual ~jive_test_value_type() noexcept;

	jive_test_value_type() noexcept;
};

class jive_test_value_input final : public jive_value_input {
public:
	virtual ~jive_test_value_input() noexcept;

	jive_test_value_input(jive_node * node, size_t index, jive_output * output);

	virtual const jive_test_value_type & type() const noexcept { return type_; }

private:
	jive_test_value_type type_;
};

class jive_test_value_output final : public jive_value_output {
public:
	virtual ~jive_test_value_output() noexcept;

	jive_test_value_output(jive_node * node, size_t index);

	virtual const jive_test_value_type & type() const noexcept { return type_; }

private:
	jive_test_value_type type_;
};

typedef struct jive_test_value_gate jive_test_value_gate;

extern const jive_gate_class JIVE_TEST_VALUE_GATE;
class jive_test_value_gate final : public jive_value_gate {
public:
	virtual ~jive_test_value_gate() noexcept;

	jive_test_value_gate(jive_graph * graph, const char name[]);

	virtual const jive_test_value_type & type() const noexcept { return type_; }

private:
	jive_test_value_type type_;
};

/* test state type */

typedef struct jive_test_state_type jive_test_state_type;

extern const jive_type_class JIVE_TEST_STATE_TYPE;
#define JIVE_DECLARE_TEST_STATE_TYPE(name) \
	jive_test_state_type name##_struct; name##_struct.class_ = &JIVE_TEST_STATE_TYPE; \
	const jive_type * name = &name##_struct

class jive_test_state_type final : public jive_state_type {
public:
	virtual ~jive_test_state_type() noexcept;

	jive_test_state_type() noexcept;
};

class jive_test_state_input final : public jive_state_input {
public:
	virtual ~jive_test_state_input() noexcept;

	jive_test_state_input(jive_node * node, size_t index, jive_output * origin);

	virtual const jive_test_state_type & type() const noexcept { return type_; }

private:
	jive_test_state_type type_;
};

class jive_test_state_output final : public jive_state_output {
public:
	virtual ~jive_test_state_output() noexcept;

	jive_test_state_output(jive_node * node, size_t index);

	virtual const jive_test_state_type & type() const noexcept { return type_; }

private:
	jive_test_state_type type_;
};

typedef struct jive_test_state_gate jive_test_state_gate;

extern const jive_gate_class JIVE_TEST_STATE_GATE;
class jive_test_state_gate final : public jive_state_gate {
public:
	virtual ~jive_test_state_gate() noexcept;

	jive_test_state_gate(jive_graph * graph, const char name[]);

	virtual const jive_test_state_type & type() const noexcept { return type_; }

private:
	jive_test_state_type type_;
};

#endif
