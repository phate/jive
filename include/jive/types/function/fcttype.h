/*
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FUNCTION_FCTTYPE_H
#define JIVE_TYPES_FUNCTION_FCTTYPE_H

#include <jive/vsdg/valuetype.h>

typedef struct jive_function_type jive_function_type;
typedef struct jive_function_input jive_function_input;
typedef struct jive_function_output jive_function_output;
typedef struct jive_function_gate jive_function_gate;

extern const jive_type_class JIVE_FUNCTION_TYPE;

struct jive_function_type {
	jive_value_type base;
	struct jive_context * ctx;
	size_t nreturns;
	jive_type ** return_types;
	size_t narguments;
	jive_type ** argument_types;
};

extern const jive_input_class JIVE_FUNCTION_INPUT;
struct jive_function_input {
	jive_value_input base;
	jive_function_type type;
};

extern const jive_output_class JIVE_FUNCTION_OUTPUT;
struct jive_function_output {
	jive_value_output base;
	jive_function_type type;
};

extern const jive_gate_class JIVE_FUNCTION_GATE;
struct jive_function_gate {
	jive_value_gate base;
	jive_function_type type;
};

jive_function_type *
jive_function_type_create(struct jive_context * context,
	size_t narguments, const jive_type * const argument_types[],
	size_t nreturns, const jive_type * const return_types[]);

void
jive_function_type_init(
	jive_function_type * self,
	struct jive_context * context,
	size_t narguments, const jive_type * const argument_types[],
	size_t nreturns, const jive_type * const return_types[]);

void
jive_function_type_fini(jive_function_type * self);

void
jive_function_type_destroy(jive_function_type * type);

#endif
