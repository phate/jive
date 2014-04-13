/*
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_UNION_UNNTYPE_H
#define JIVE_TYPES_UNION_UNNTYPE_H

#include <jive/vsdg/valuetype.h>

/* union declaration */

typedef struct jive_union_declaration jive_union_declaration;

struct jive_union_declaration {
	size_t nelements;
	const jive_value_type ** elements;
};

/* union type */

typedef struct jive_union_type jive_union_type;

extern const jive_type_class JIVE_UNION_TYPE;

struct jive_union_type : public jive_value_type {
	const jive_union_declaration * decl;
};

#define JIVE_DECLARE_UNION_TYPE(name, decl) \
	jive_union_type name##_struct; \
	name##_struct.class_ = &JIVE_UNION_TYPE; name##_struct.decl = decl; \
	const jive_type * name = &name##_struct

void
jive_union_type_init(jive_union_type * self, const jive_union_declaration * decl);

JIVE_EXPORTED_INLINE const jive_union_type *
jive_union_type_const_cast(const jive_type * self)
{
	if (jive_type_isinstance(self, &JIVE_UNION_TYPE))
		return (const jive_union_type *)self;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_union_type *
jive_union_type_cast(jive_type * self)
{
	if (jive_type_isinstance(self, &JIVE_UNION_TYPE))
		return (jive_union_type *)self;
	else
		return NULL;
}

/* union input */

typedef struct jive_union_input jive_union_input;

extern const jive_input_class JIVE_UNION_INPUT;
struct jive_union_input : public jive_value_input {
	jive_union_type type;
};

JIVE_EXPORTED_INLINE const jive_union_input *
jive_union_input_const_cast(const jive_input * self)
{
	if (jive_input_isinstance(self, &JIVE_UNION_INPUT))
		return (const jive_union_input *)self;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_union_input *
jive_union_input_cast(jive_input * self)
{
	if (jive_input_isinstance(self, &JIVE_UNION_INPUT))
		return (jive_union_input *)self;
	else
		return NULL;
}

/* union output */

typedef struct jive_union_output jive_union_output;

extern const jive_output_class JIVE_UNION_OUTPUT;
struct jive_union_output : public jive_value_output {
	jive_union_type type;
};

JIVE_EXPORTED_INLINE const jive_union_output *
jive_union_output_const_cast(const jive_output * self)
{
	if (jive_output_isinstance(self, &JIVE_UNION_OUTPUT))
		return (const jive_union_output *)self;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_union_output *
jive_union_output_cast(jive_output * self)
{
	if (jive_output_isinstance(self, &JIVE_UNION_OUTPUT))
		return (jive_union_output *)self;
	else
		return NULL;
}

/* union gate */

typedef struct jive_union_gate jive_union_gate;

extern const jive_gate_class JIVE_UNION_GATE;
struct jive_union_gate : public jive_value_gate {
	jive_union_type type;
};

JIVE_EXPORTED_INLINE const jive_union_gate *
jive_union_gate_const_cast(const jive_gate * self)
{
	if (jive_gate_isinstance(self, &JIVE_UNION_GATE))
		return (const jive_union_gate *)self;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_union_gate *
jive_union_gate_cast(jive_gate * self)
{
	if (jive_gate_isinstance(self, &JIVE_UNION_GATE))
		return (jive_union_gate *)self;
	else
		return NULL;
}

#endif
