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

struct jive_test_value_type : public jive_value_type {
};

typedef struct jive_test_value_input jive_test_value_input;

extern const jive_input_class JIVE_TEST_VALUE_INPUT;
struct jive_test_value_input : public jive_value_input {
	jive_test_value_type type;
};

typedef struct jive_test_value_output jive_test_value_output;

extern const jive_output_class JIVE_TEST_VALUE_OUTPUT;
struct jive_test_value_output : public jive_value_output {
	jive_test_value_type type;
};

typedef struct jive_test_value_gate jive_test_value_gate;

extern const jive_gate_class JIVE_TEST_VALUE_GATE;
struct jive_test_value_gate : public jive_value_gate {
	jive_test_value_type type;
};

/* test state type */

typedef struct jive_test_state_type jive_test_state_type;

extern const jive_type_class JIVE_TEST_STATE_TYPE;
#define JIVE_DECLARE_TEST_STATE_TYPE(name) \
	jive_test_state_type name##_struct; name##_struct.class_ = &JIVE_TEST_STATE_TYPE; \
	const jive_type * name = &name##_struct

struct jive_test_state_type : public jive_state_type {
};

typedef struct jive_test_state_input jive_test_state_input;

extern const jive_input_class JIVE_TEST_STATE_INPUT;
struct jive_test_state_input : public jive_state_input {
	jive_test_state_type type;
};

typedef struct jive_test_state_output jive_test_state_output;

extern const jive_output_class JIVE_TEST_STATE_OUTPUT;
struct jive_test_state_output : public jive_state_output {
	jive_test_state_type type;
};

typedef struct jive_test_state_gate jive_test_state_gate;

extern const jive_gate_class JIVE_TEST_STATE_GATE;
struct jive_test_state_gate : public jive_state_gate {
	jive_test_state_type type;
};

#endif
