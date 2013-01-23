/*
 * Copyright 2013 Helge Bahmann <hcb@chaoticmind.net>
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
	const jive_immediate_type name##_struct = {{{&JIVE_IMMEDIATE_TYPE}}}; \
	const jive_type * name = &name##_struct.base.base

struct jive_immediate_type {
	jive_value_type base;
};

extern const jive_input_class JIVE_IMMEDIATE_INPUT;
struct jive_immediate_input {
	jive_value_input base;
	jive_immediate_type type;
};

extern const jive_output_class JIVE_IMMEDIATE_OUTPUT;
struct jive_immediate_output {
	jive_value_output base;
	jive_immediate_type type;
};

extern const jive_gate_class JIVE_IMMEDIATE_GATE;
struct jive_immediate_gate {
	jive_value_gate base;
	jive_immediate_type type;
};

#endif
