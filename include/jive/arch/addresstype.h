/*
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_ADDRESSTYPE_H
#define JIVE_ARCH_ADDRESSTYPE_H

#include <jive/vsdg/valuetype.h>

/* address type */

typedef struct jive_address_type jive_address_type;

extern const jive_type_class JIVE_ADDRESS_TYPE;
#define JIVE_DECLARE_ADDRESS_TYPE(name) \
	const jive_address_type name##_struct = {{{&JIVE_ADDRESS_TYPE}}}; \
	const jive_type * name = &name##_struct.base.base

struct jive_address_type {
	jive_value_type base;
};

void
jive_address_type_init(jive_address_type * self);

JIVE_EXPORTED_INLINE const jive_address_type *
jive_address_type_const_cast(const jive_type * self)
{
	if (jive_type_isinstance(self, &JIVE_ADDRESS_TYPE))
		return (const jive_address_type *)self;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_address_type *
jive_address_type_cast(jive_type * self)
{
	if (jive_type_isinstance(self, &JIVE_ADDRESS_TYPE))
		return (jive_address_type *)self;
	else
		return NULL;
}

/* address input */

typedef struct jive_address_input jive_address_input;

extern const jive_input_class JIVE_ADDRESS_INPUT;
struct jive_address_input {
	jive_value_input base;
	jive_address_type type;
};

JIVE_EXPORTED_INLINE const jive_address_input *
jive_address_input_const_cast(const jive_input * self)
{
	if (jive_input_isinstance(self, &JIVE_ADDRESS_INPUT))
		return (const jive_address_input *)self;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_address_input *
jive_address_input_cast(jive_input * self)
{
	if (jive_input_isinstance(self, &JIVE_ADDRESS_INPUT))
		return (jive_address_input *)self;
	else
		return NULL;
}

/* address output */

typedef struct jive_address_output jive_address_output;

extern const jive_output_class JIVE_ADDRESS_OUTPUT;
struct jive_address_output {
	jive_value_output base;
	jive_address_type type;
};

JIVE_EXPORTED_INLINE const jive_address_output *
jive_address_output_const_cast(const jive_output * self)
{
	if (jive_output_isinstance(self, &JIVE_ADDRESS_OUTPUT))
		return (const jive_address_output *)self;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_address_output *
jive_address_output_cast(jive_output * self)
{
	if (jive_output_isinstance(self, &JIVE_ADDRESS_OUTPUT))
		return (jive_address_output *)self;
	else
		return NULL;
}

/* address gate */

typedef struct jive_address_gate jive_address_gate;

extern const jive_gate_class JIVE_ADDRESS_GATE;
struct jive_address_gate {
	jive_value_gate base;
	jive_address_type type;
};

JIVE_EXPORTED_INLINE const jive_address_gate *
jive_address_gate_const_cast(const jive_gate * self)
{
	if (jive_gate_isinstance(self, &JIVE_ADDRESS_GATE))
		return (const jive_address_gate *)self;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_address_gate *
jive_address_gate_cast(jive_gate * self)
{
	if (jive_gate_isinstance(self, &JIVE_ADDRESS_GATE))
		return (jive_address_gate *)self;
	else
		return NULL;
}

#endif
