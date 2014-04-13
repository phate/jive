/*
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_TYPE_H
#define JIVE_TYPES_BITSTRING_TYPE_H

#include <jive/vsdg/valuetype.h>

/* bitstring type */

typedef struct jive_bitstring_type jive_bitstring_type;

extern const jive_type_class JIVE_BITSTRING_TYPE;
#define JIVE_DECLARE_BITSTRING_TYPE(name, nbits) \
	jive_bitstring_type name##_struct(nbits); \
	const jive_type * name = &name##_struct

struct jive_bitstring_type : public jive_value_type {
	jive_bitstring_type(size_t nbits_) : nbits(nbits_) { class_ = &JIVE_BITSTRING_TYPE; }
	size_t nbits;
};

JIVE_EXPORTED_INLINE jive_bitstring_type *
jive_bitstring_type_cast(jive_type * type)
{
	if (jive_type_isinstance(type, &JIVE_BITSTRING_TYPE))
		return (jive_bitstring_type *) type;
	else
		return 0;
}

JIVE_EXPORTED_INLINE const jive_bitstring_type *
jive_bitstring_type_const_cast(const jive_type * type)
{
	if (jive_type_isinstance(type, &JIVE_BITSTRING_TYPE))
		return (const jive_bitstring_type *) type;
	else
		return 0;
}


/* bitstring input */

typedef struct jive_bitstring_input jive_bitstring_input;

extern const jive_input_class JIVE_BITSTRING_INPUT;
struct jive_bitstring_input : public jive_value_input {
	jive_bitstring_type type;
};

JIVE_EXPORTED_INLINE jive_bitstring_input *
jive_bitstring_input_cast(jive_input * input)
{
	if (jive_input_isinstance(input, &JIVE_BITSTRING_INPUT))
		return (jive_bitstring_input *) input;
	else
		return 0;
}

JIVE_EXPORTED_INLINE const jive_bitstring_input *
jive_bitstring_input_const_cast(const jive_input * input)
{
	if (jive_input_isinstance(input, &JIVE_BITSTRING_INPUT))
		return (const jive_bitstring_input *) input;
	else
		return 0;
}

JIVE_EXPORTED_INLINE size_t
jive_bitstring_input_nbits(const jive_bitstring_input * self)
{
	return self->type.nbits;
}

/* bitstring output */

typedef struct jive_bitstring_output jive_bitstring_output;

extern const jive_output_class JIVE_BITSTRING_OUTPUT;
struct jive_bitstring_output {
	jive_value_output base;
	jive_bitstring_type type;
};

JIVE_EXPORTED_INLINE jive_bitstring_output *
jive_bitstring_output_cast(jive_output * output)
{
	if (jive_output_isinstance(output, &JIVE_BITSTRING_OUTPUT))
		return (jive_bitstring_output *) output;
	else
		return 0;
}

JIVE_EXPORTED_INLINE const jive_bitstring_output *
jive_bitstring_output_const_cast(const jive_output * output)
{
	if (jive_output_isinstance(output, &JIVE_BITSTRING_OUTPUT))
		return (const jive_bitstring_output *) output;
	else
		return 0;
}

JIVE_EXPORTED_INLINE size_t
jive_bitstring_output_nbits(const jive_bitstring_output * self)
{
	return self->type.nbits;
}

/* bitstring gate */

typedef struct jive_bitstring_gate jive_bitstring_gate;

extern const jive_gate_class JIVE_BITSTRING_GATE;
struct jive_bitstring_gate {
	jive_value_gate base;
	jive_bitstring_type type;
};

JIVE_EXPORTED_INLINE jive_bitstring_gate *
jive_bitstring_gate_cast(jive_gate * gate)
{
	if (jive_gate_isinstance(gate, &JIVE_BITSTRING_GATE))
		return (jive_bitstring_gate *) gate;
	else
		return 0;
}

JIVE_EXPORTED_INLINE const jive_bitstring_gate *
jive_bitstring_gate_const_cast(const jive_gate * gate)
{
	if (jive_gate_isinstance(gate, &JIVE_BITSTRING_GATE))
		return (const jive_bitstring_gate *) gate;
	else
		return 0;
}

JIVE_EXPORTED_INLINE size_t
jive_bitstring_gate_nbits(const jive_bitstring_gate * self)
{
	return self->type.nbits;
}

#endif
