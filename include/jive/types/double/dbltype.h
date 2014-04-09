/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_DOUBLE_DBLTYPE_H
#define JIVE_TYPES_DOUBLE_DBLTYPE_H

#include <jive/vsdg/valuetype.h>

/* double type */

typedef struct jive_double_type jive_double_type;

extern const jive_type_class JIVE_DOUBLE_TYPE;
#define JIVE_DECLARE_DOUBLE_TYPE(name) \
	jive_double_type name##_struct; name##_struct.class_ = &JIVE_DOUBLE_TYPE; \
	const jive_type * name = &name##_struct

struct jive_double_type : public jive_value_type {
};

JIVE_EXPORTED_INLINE const jive_double_type *
jive_double_type_const_cast(const jive_type * self)
{
	if (jive_type_isinstance(self, &JIVE_DOUBLE_TYPE))
		return (const jive_double_type *)self;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_double_type *
jive_double_type_cast(jive_type * self)
{
	if (jive_type_isinstance(self, &JIVE_DOUBLE_TYPE))
		return (jive_double_type *)self;
	else
		return NULL;
}

/* double input */

typedef struct jive_double_input jive_double_input;

extern const jive_input_class JIVE_DOUBLE_INPUT;
struct jive_double_input {
	jive_value_input base;
	jive_double_type type;
};

JIVE_EXPORTED_INLINE const jive_double_input *
jive_double_input_const_cast(const jive_input * self)
{
	if (jive_input_isinstance(self, &JIVE_DOUBLE_INPUT))
		return (const jive_double_input *)self;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_double_input *
jive_double_input_cast(jive_input * self)
{
	if (jive_input_isinstance(self, &JIVE_DOUBLE_INPUT))
		return (jive_double_input *)self;
	else
		return NULL;
}

/* double output */

typedef struct jive_double_output jive_double_output;

extern const jive_output_class JIVE_DOUBLE_OUTPUT;
struct jive_double_output {
	jive_value_output base;
	jive_double_type type;
};

JIVE_EXPORTED_INLINE const jive_double_output *
jive_double_output_const_cast(const jive_output * self)
{
	if (jive_output_isinstance(self, &JIVE_DOUBLE_OUTPUT))
		return (const jive_double_output *)self;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_double_output *
jive_double_output_cast(jive_output * self)
{
	if (jive_output_isinstance(self, &JIVE_DOUBLE_OUTPUT))
		return (jive_double_output *)self;
	else
		return NULL;
}

/* double gate */

typedef struct jive_double_gate jive_double_gate;

extern const jive_gate_class JIVE_DOUBLE_GATE;
struct jive_double_gate {
	jive_value_gate base;
	jive_double_type type;
};

JIVE_EXPORTED_INLINE const jive_double_gate *
jive_double_gate_const_cast(const jive_gate * self)
{
	if (jive_gate_isinstance(self, &JIVE_DOUBLE_GATE))
		return (const jive_double_gate *)self;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_double_gate *
jive_double_gate_cast(jive_gate * self)
{
	if (jive_gate_isinstance(self, &JIVE_DOUBLE_GATE))
		return (jive_double_gate *)self;
	else
		return NULL;
}

#endif
