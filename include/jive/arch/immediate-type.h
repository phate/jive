/*
 * Copyright 2013 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_IMMEDIATE_TYPE_H
#define JIVE_ARCH_IMMEDIATE_TYPE_H

#include <jive/vsdg/valuetype.h>

typedef struct jive_immediate_gate jive_immediate_gate;
typedef struct jive_immediate_input jive_immediate_input;
typedef struct jive_immediate_output jive_immediate_output;
typedef struct jive_immediate_type jive_immediate_type;

extern const jive_type_class JIVE_IMMEDIATE_TYPE;
#define JIVE_DECLARE_IMMEDIATE_TYPE(name) \
	jive_immediate_type name##_struct; name##_struct.class_ = &JIVE_IMMEDIATE_TYPE; \
	const jive_type * name = &name##_struct

struct jive_immediate_type : public jive_value_type {
};

extern const jive_input_class JIVE_IMMEDIATE_INPUT;
struct jive_immediate_input : public jive_value_input {
	jive_immediate_type type;
};

extern const jive_output_class JIVE_IMMEDIATE_OUTPUT;
struct jive_immediate_output : public jive_value_output {
	jive_immediate_type type;
};

extern const jive_gate_class JIVE_IMMEDIATE_GATE;
struct jive_immediate_gate : public jive_value_gate {
	jive_immediate_type type;
};

#endif
