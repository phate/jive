/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_REAL_RLTYPE_H
#define JIVE_TYPES_REAL_RLTYPE_H

#include <jive/vsdg/valuetype.h>

/* real type */

typedef struct jive_real_type jive_real_type;

extern const jive_type_class JIVE_REAL_TYPE;
#define JIVE_DECLARE_REAL_TYPE(name) \
	const jive_real_type name##_struct = {{{&JIVE_REAL_TYPE}}}; \
	const jive_type * name = &name##_struct.base.base

struct jive_real_type {
	jive_value_type base;
};

JIVE_EXPORTED_INLINE struct jive_real_type *
jive_real_type_cast(struct jive_type * type)
{
	if (jive_type_isinstance(type, &JIVE_REAL_TYPE))
		return (struct jive_real_type *) type;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_real_type *
jive_real_type_const_cast(const struct jive_type * type)
{
	if (jive_type_isinstance(type, &JIVE_REAL_TYPE))
		return (const struct jive_real_type *) type;
	else
		return NULL;
}

/* real input */

typedef struct jive_real_input jive_real_input;

extern const jive_input_class JIVE_REAL_INPUT;
struct jive_real_input {
	jive_value_input base;
	jive_real_type type;
};

JIVE_EXPORTED_INLINE struct jive_real_input *
jive_real_input_cast(struct jive_input * input)
{
	if (jive_input_isinstance(input, &JIVE_REAL_INPUT))
		return (struct jive_real_input *) input;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_real_input *
jive_real_input_const_cast(const struct jive_input * input)
{
	if (jive_input_isinstance(input, &JIVE_REAL_INPUT))
		return (const struct jive_real_input *) input;
	else
		return NULL;
}

/* real output */

typedef struct jive_real_output jive_real_output;

extern const jive_output_class JIVE_REAL_OUTPUT;
struct jive_real_output {
	jive_value_output base;
	jive_real_type type;
};

JIVE_EXPORTED_INLINE struct jive_real_output *
jive_real_output_cast(struct jive_output * output)
{
	if (jive_output_isinstance(output, &JIVE_REAL_OUTPUT))
		return (struct jive_real_output *) output;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_real_output *
jive_real_output_const_cast(const struct jive_output * output)
{
	if (jive_output_isinstance(output, &JIVE_REAL_OUTPUT))
		return (const struct jive_real_output *) output;
	else
		return NULL;
}

/* real gate */

typedef struct jive_real_gate jive_real_gate;

extern const jive_gate_class JIVE_REAL_GATE;
struct jive_real_gate {
	jive_value_gate base;
	jive_real_type type;
};

JIVE_EXPORTED_INLINE struct jive_real_gate *
jive_real_gate_cast(struct jive_gate * gate)
{
	if (jive_gate_isinstance(gate, &JIVE_REAL_GATE))
		return (struct jive_real_gate *) gate;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_real_gate *
jive_real_gate_const_cast(const struct jive_gate * gate)
{
	if (jive_gate_isinstance(gate, &JIVE_REAL_GATE))
		return (const struct jive_real_gate *) gate;
	else
		return NULL;
}

#endif
