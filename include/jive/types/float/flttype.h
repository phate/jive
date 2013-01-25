/*
 * Copyright 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_FLTTYPE_H
#define JIVE_TYPES_FLOAT_FLTTYPE_H

#include <jive/vsdg/valuetype.h>

typedef struct jive_float_type jive_float_type;
typedef struct jive_float_input jive_float_input;
typedef struct jive_float_output jive_float_output;
typedef struct jive_float_gate jive_float_gate;

extern const jive_type_class JIVE_FLOAT_TYPE;
#define JIVE_DECLARE_FLOAT_TYPE(name) \
	const jive_float_type name##_struct = {{{&JIVE_FLOAT_TYPE}}}; \
	const jive_type * name = &name##_struct.base.base

struct jive_float_type {
	jive_value_type base;
}; 

extern const jive_input_class JIVE_FLOAT_INPUT;
struct jive_float_input {
	jive_value_input base;
	jive_float_type type;
};

extern const jive_output_class JIVE_FLOAT_OUTPUT;
struct jive_float_output {
	jive_value_output base;
	jive_float_type type;
};

extern const jive_gate_class JIVE_FLOAT_GATE;
struct jive_float_gate {
	jive_value_gate base;
	jive_float_type type;
};

#endif
