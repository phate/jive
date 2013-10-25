/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_INTEGRAL_ITGTYPE_H
#define JIVE_TYPES_INTEGRAL_ITGTYPE_H

#include <jive/vsdg/valuetype.h>

/* integral type */

typedef struct jive_integral_type jive_integral_type;

extern const jive_type_class JIVE_INTEGRAL_TYPE;
#define JIVE_DECLARE_INTEGRAL_TYPE(name) \
	const jive_integral_type name##_struct = {{{&JIVE_INTEGRAL_TYPE}}}; \
	const jive_type * name = &name##_struct.base.base

struct jive_integral_type {
	jive_value_type base;
};

JIVE_EXPORTED_INLINE struct jive_integral_type *
jive_integral_type_cast(struct jive_type * type)
{
	if (jive_type_isinstance(type, &JIVE_INTEGRAL_TYPE))
		return (struct jive_integral_type *) type;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_integral_type *
jive_integral_type_const_cast(const struct jive_type * type)
{
	if (jive_type_isinstance(type, &JIVE_INTEGRAL_TYPE))
		return (const struct jive_integral_type *) type;
	else
		return NULL;
}

/* integral input */

typedef struct jive_integral_input jive_integral_input;

extern const jive_input_class JIVE_INTEGRAL_INPUT;
struct jive_integral_input {
	jive_value_input base;
	jive_integral_type type;
};

JIVE_EXPORTED_INLINE struct jive_integral_input *
jive_integral_input_cast(struct jive_input * input)
{
	if (jive_input_isinstance(input, &JIVE_INTEGRAL_INPUT))
		return (struct jive_integral_input *) input;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_integral_input *
jive_integral_input_const_cast(const struct jive_input * input)
{
	if (jive_input_isinstance(input, &JIVE_INTEGRAL_INPUT))
		return (const struct jive_integral_input *) input;
	else
		return NULL;
}

/* integral output */

typedef struct jive_integral_output jive_integral_output;

extern const jive_output_class JIVE_INTEGRAL_OUTPUT;
struct jive_integral_output {
	jive_value_output base;
	jive_integral_type type;
};

JIVE_EXPORTED_INLINE struct jive_integral_output *
jive_integral_output_cast(struct jive_output * output)
{
	if (jive_output_isinstance(output, &JIVE_INTEGRAL_OUTPUT))
		return (struct jive_integral_output *) output;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_integral_output *
jive_integral_output_const_cast(const struct jive_output * output)
{
	if (jive_output_isinstance(output, &JIVE_INTEGRAL_OUTPUT))
		return (const struct jive_integral_output *) output;
	else
		return NULL;
}

/* integral gate */

typedef struct jive_integral_gate jive_integral_gate;

extern const jive_gate_class JIVE_INTEGRAL_GATE;
struct jive_integral_gate {
	jive_value_gate base;
	jive_integral_type type;
};

JIVE_EXPORTED_INLINE struct jive_integral_gate *
jive_integral_gate_cast(struct jive_gate * gate)
{
	if (jive_gate_isinstance(gate, &JIVE_INTEGRAL_GATE))
		return (struct jive_integral_gate *) gate;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_integral_gate *
jive_integral_gate_const_cast(const struct jive_gate * gate)
{
	if (jive_gate_isinstance(gate, &JIVE_INTEGRAL_GATE))
		return (const struct jive_integral_gate *) gate;
	else
		return NULL;
}

#endif
