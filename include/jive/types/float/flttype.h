/*
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_FLTTYPE_H
#define JIVE_TYPES_FLOAT_FLTTYPE_H

#include <jive/vsdg/valuetype.h>

/* float type */

typedef struct jive_float_type jive_float_type;

extern const jive_type_class JIVE_FLOAT_TYPE;
#define JIVE_DECLARE_FLOAT_TYPE(name) \
	jive_float_type name##_struct; name##_struct.class_ = &JIVE_FLOAT_TYPE; \
	const jive_type * name = &name##_struct

struct jive_float_type : public jive_value_type {
	jive_float_type() { class_ = &JIVE_FLOAT_TYPE; }
};

JIVE_EXPORTED_INLINE const jive_float_type *
jive_float_type_const_cast(const jive_type * self)
{
	if (jive_type_isinstance(self, &JIVE_FLOAT_TYPE))
		return (const jive_float_type *)self;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_float_type *
jive_float_type_cast(jive_type * self)
{
	if (jive_type_isinstance(self, &JIVE_FLOAT_TYPE))
		return (jive_float_type *)self;
	else
		return NULL;
}

/* float input */

typedef struct jive_float_input jive_float_input;

extern const jive_input_class JIVE_FLOAT_INPUT;
struct jive_float_input : public jive_value_input {
	jive_float_type type;
};

JIVE_EXPORTED_INLINE const jive_float_input *
jive_float_input_const_cast(const jive_input * self)
{
	if (jive_input_isinstance(self, &JIVE_FLOAT_INPUT))
		return (const jive_float_input *)self;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_float_input *
jive_float_input_cast(jive_input * self)
{
	if (jive_input_isinstance(self, &JIVE_FLOAT_INPUT))
		return (jive_float_input *)self;
	else
		return NULL;
}

/* float output */

typedef struct jive_float_output jive_float_output;

extern const jive_output_class JIVE_FLOAT_OUTPUT;
struct jive_float_output : public jive_value_output {
	jive_float_type type;
};

JIVE_EXPORTED_INLINE const jive_float_output *
jive_float_output_const_cast(const jive_output * self)
{
	if (jive_output_isinstance(self, &JIVE_FLOAT_OUTPUT))
		return (const jive_float_output *)self;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_float_output *
jive_float_output_cast(jive_output * self)
{
	if (jive_output_isinstance(self, &JIVE_FLOAT_OUTPUT))
		return (jive_float_output *)self;
	else
		return NULL;
}

/* float gate */

typedef struct jive_float_gate jive_float_gate;

extern const jive_gate_class JIVE_FLOAT_GATE;
struct jive_float_gate {
	jive_value_gate base;
	jive_float_type type;
};

JIVE_EXPORTED_INLINE const jive_float_gate *
jive_float_gate_const_cast(const jive_gate * self)
{
	if (jive_gate_isinstance(self, &JIVE_FLOAT_GATE))
		return (const jive_float_gate *)self;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_float_gate *
jive_float_gate_cast(jive_gate * self)
{
	if (jive_gate_isinstance(self, &JIVE_FLOAT_GATE))
		return (jive_float_gate *)self;
	else
		return NULL;
}

#endif
