/*
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_ADDRESSTYPE_H
#define JIVE_ARCH_ADDRESSTYPE_H

#include <jive/vsdg/valuetype.h>

typedef struct jive_address_type jive_address_type;
typedef struct jive_address_input jive_address_input;
typedef struct jive_address_output jive_address_output;
typedef struct jive_address_gate jive_address_gate;

extern const jive_type_class JIVE_ADDRESS_TYPE;
#define JIVE_DECLARE_ADDRESS_TYPE(name) \
	const jive_address_type name##_struct = {{{&JIVE_ADDRESS_TYPE}}}; \
	const jive_type * name = &name##_struct.base.base

struct jive_address_type {
	jive_value_type base;
};

extern const jive_input_class JIVE_ADDRESS_INPUT;
struct jive_address_input {
	jive_value_input base;
	jive_address_type type;
};

extern const jive_output_class JIVE_ADDRESS_OUTPUT;
struct jive_address_output {
	jive_value_output base;
	jive_address_type type;
};

extern const jive_gate_class JIVE_ADDRESS_GATE;
struct jive_address_gate {
	jive_value_gate base;
	jive_address_type type;
};

void
jive_address_type_init(jive_address_type * self);

#endif
