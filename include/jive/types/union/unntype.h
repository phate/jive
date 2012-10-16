/*
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_UNION_UNNTYPE_H
#define JIVE_TYPES_UNION_UNNTYPE_H

#include <jive/vsdg/valuetype.h>

typedef struct jive_union_type jive_union_type;
typedef struct jive_union_input jive_union_input;
typedef struct jive_union_output jive_union_output;
typedef struct jive_union_gate jive_union_gate;

typedef struct jive_union_declaration jive_union_declaration;

struct jive_union_declaration {
	size_t nelements;
	const jive_value_type ** elements;
};

extern const jive_type_class JIVE_UNION_TYPE;

struct jive_union_type {
	jive_value_type base;
	const jive_union_declaration * decl;
};

#define JIVE_DECLARE_UNION_TYPE(name, decl) \
	const jive_union_type name##_struct = {{{&JIVE_UNION_TYPE}}, decl}; \
	const jive_type * name = &name##_struct.base.base

extern const jive_input_class JIVE_UNION_INPUT;
struct jive_union_input {
	jive_value_input base;
	jive_union_type type;
};

extern const jive_output_class JIVE_UNION_OUTPUT;
struct jive_union_output {
	jive_value_output base;
	jive_union_type type;
};

extern const jive_gate_class JIVE_UNION_GATE;
struct jive_union_gate {
	jive_value_gate base;
	jive_union_type type;
};

void
jive_union_type_init(
	jive_union_type * self,
	const jive_union_declaration * decl);

#endif
