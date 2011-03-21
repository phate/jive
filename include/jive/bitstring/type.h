#ifndef JIVE_BITSTRING_TYPE_H
#define JIVE_BITSTRING_TYPE_H

#include <jive/vsdg/valuetype.h>

typedef struct jive_bitstring_type jive_bitstring_type;
typedef struct jive_bitstring_input jive_bitstring_input;
typedef struct jive_bitstring_output jive_bitstring_output;
typedef struct jive_bitstring_gate jive_bitstring_gate;

extern const jive_type_class JIVE_BITSTRING_TYPE;
#define JIVE_DECLARE_BITSTRING_TYPE(name, nbits) \
	const jive_bitstring_type name##_struct = {{{&JIVE_BITSTRING_TYPE}}, nbits}; \
	const jive_type * name = &name##_struct.base.base

struct jive_bitstring_type {
	jive_value_type base;
	size_t nbits;
};

/* TODO: definitions for value range propagation */

extern const jive_input_class JIVE_BITSTRING_INPUT;
struct jive_bitstring_input {
	jive_value_input base;
	jive_bitstring_type type;
};

extern const jive_output_class JIVE_BITSTRING_OUTPUT;
struct jive_bitstring_output {
	jive_value_output base;
	jive_bitstring_type type;
};

extern const jive_gate_class JIVE_BITSTRING_GATE;
struct jive_bitstring_gate {
	jive_value_gate base;
	jive_bitstring_type type;
};

static inline size_t
jive_bitstring_input_nbits(const jive_bitstring_input * self)
{
	return self->type.nbits;
}

static inline size_t
jive_bitstring_output_nbits(const jive_bitstring_output * self)
{
	return self->type.nbits;
}

static inline size_t
jive_bitstring_gate_nbits(const jive_bitstring_gate * self)
{
	return self->type.nbits;
}

#endif
