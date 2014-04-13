/*
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_RECORD_RCDTYPE_H
#define JIVE_TYPES_RECORD_RCDTYPE_H

#include <jive/vsdg/valuetype.h>

/* record declaration */

typedef struct jive_record_declaration jive_record_declaration;

struct jive_record_declaration {
	size_t nelements;
	const jive_value_type ** elements;
};

/* record type */

typedef struct jive_record_type jive_record_type;

extern const jive_type_class JIVE_RECORD_TYPE;
struct jive_record_type : public jive_value_type {
	const jive_record_declaration * decl;
};

void
jive_record_type_init(jive_record_type * self, const jive_record_declaration * decl);

JIVE_EXPORTED_INLINE const jive_record_type *
jive_record_type_const_cast(const jive_type * self)
{
	if (jive_type_isinstance(self, &JIVE_RECORD_TYPE))
		return (const jive_record_type *)self;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_record_type *
jive_record_type_cast(jive_type * self)
{
	if (jive_type_isinstance(self, &JIVE_RECORD_TYPE))
		return (jive_record_type *)self;
	else
		return NULL;
}

/* record input */

typedef struct jive_record_input jive_record_input;

extern const jive_input_class JIVE_RECORD_INPUT;
struct jive_record_input : public jive_value_input {
	jive_record_type type;
};

JIVE_EXPORTED_INLINE const jive_record_input *
jive_record_input_const_cast(const jive_input * self)
{
	if (jive_input_isinstance(self, &JIVE_RECORD_INPUT))
		return (const jive_record_input *)self;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_record_input *
jive_record_input_cast(jive_input * self)
{
	if (jive_input_isinstance(self, &JIVE_RECORD_INPUT))
		return (jive_record_input *)self;
	else
		return NULL;
}

/* record output */

typedef struct jive_record_output jive_record_output;

extern const jive_output_class JIVE_RECORD_OUTPUT;
struct jive_record_output : public jive_value_output {
	jive_record_type type;
};

JIVE_EXPORTED_INLINE const jive_record_output *
jive_record_output_const_cast(const jive_output * self)
{
	if (jive_output_isinstance(self, &JIVE_RECORD_OUTPUT))
		return (const jive_record_output *)self;
	else
		return 0;
}

JIVE_EXPORTED_INLINE jive_record_output *
jive_record_output_cast(jive_output * self)
{
	if (jive_output_isinstance(self, &JIVE_RECORD_OUTPUT))
		return (jive_record_output *)self;
	else
		return 0;
}

/* record gate */

typedef struct jive_record_gate jive_record_gate;

extern const jive_gate_class JIVE_RECORD_GATE;
struct jive_record_gate {
	jive_value_gate base;
	jive_record_type type;
};

JIVE_EXPORTED_INLINE const jive_record_gate *
jive_record_gate_const_cast(const jive_gate * self)
{
	if (jive_gate_isinstance(self, &JIVE_RECORD_GATE))
		return (const jive_record_gate *)self;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_record_gate *
jive_record_gate_cast(jive_gate * self)
{
	if (jive_gate_isinstance(self, &JIVE_RECORD_GATE))
		return (jive_record_gate *)self;
	else
		return NULL;
}

#endif
