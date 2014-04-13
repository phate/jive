/*
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FUNCTION_FCTTYPE_H
#define JIVE_TYPES_FUNCTION_FCTTYPE_H

#include <jive/vsdg/valuetype.h>

/* function type */

extern const jive_type_class JIVE_FUNCTION_TYPE;

typedef struct jive_function_type jive_function_type;
struct jive_function_type : public jive_value_type {
	struct jive_context * ctx;
	size_t nreturns;
	jive_type ** return_types;
	size_t narguments;
	jive_type ** argument_types;
};

JIVE_EXPORTED_INLINE struct jive_function_type *
jive_function_type_cast(struct jive_type * type)
{
	if (jive_type_isinstance(type, &JIVE_FUNCTION_TYPE))
		return (jive_function_type *) type;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_function_type *
jive_function_type_const_cast(const struct jive_type * type)
{
	if (jive_type_isinstance(type, &JIVE_FUNCTION_TYPE))
		return (const jive_function_type *) type;
	else
		return NULL;
}

void
jive_function_type_init(
	jive_function_type * self,
	struct jive_context * context,
	size_t narguments, const jive_type * const argument_types[],
	size_t nreturns, const jive_type * const return_types[]);

void
jive_function_type_fini(jive_function_type * self);

jive_function_type *
jive_function_type_create(struct jive_context * context,
	size_t narguments, const jive_type * const argument_types[],
	size_t nreturns, const jive_type * const return_types[]);

void
jive_function_type_destroy(jive_function_type * type);

/* function input */

extern const jive_input_class JIVE_FUNCTION_INPUT;

typedef struct jive_function_input jive_function_input;
struct jive_function_input : public jive_value_input {
	jive_function_type type;
};

JIVE_EXPORTED_INLINE struct jive_function_input *
jive_function_input_cast(struct jive_input * input)
{
	if (jive_input_isinstance(input, &JIVE_FUNCTION_INPUT))
		return (jive_function_input *) input;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_function_input *
jive_function_input_const_cast(const struct jive_input * input)
{
	if (jive_input_isinstance(input, &JIVE_FUNCTION_INPUT))
		return (const jive_function_input *) input;
	else
		return NULL;
}

/* function output */

extern const jive_output_class JIVE_FUNCTION_OUTPUT;

typedef struct jive_function_output jive_function_output;
struct jive_function_output {
	jive_value_output base;
	jive_function_type type;
};

JIVE_EXPORTED_INLINE struct jive_function_output *
jive_function_output_cast(struct jive_output * output)
{
	if (jive_output_isinstance(output, &JIVE_FUNCTION_OUTPUT))
		return (jive_function_output *) output;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_function_output *
jive_function_output_const_cast(const struct jive_output * output)
{
	if (jive_output_isinstance(output, &JIVE_FUNCTION_OUTPUT))
		return (const jive_function_output *) output;
	else
		return NULL;
}

/* function gate */

extern const jive_gate_class JIVE_FUNCTION_GATE;

typedef struct jive_function_gate jive_function_gate;
struct jive_function_gate {
	jive_value_gate base;
	jive_function_type type;
};

JIVE_EXPORTED_INLINE struct jive_function_gate *
jive_function_gate_cast(struct jive_gate * gate)
{
	if (jive_gate_isinstance(gate, &JIVE_FUNCTION_GATE))
		return (jive_function_gate *) gate;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_function_gate *
jive_function_gate_const_cast(const struct jive_gate * gate)
{
	if (jive_gate_isinstance(gate, &JIVE_FUNCTION_GATE))
		return (const jive_function_gate *) gate;
	else
		return NULL;
}

#endif
