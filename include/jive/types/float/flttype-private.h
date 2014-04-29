/*
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_FLTTYPE_PRIVATE_H
#define JIVE_TYPES_FLOAT_FLTTYPE_PRIVATE_H

#include <jive/types/float/flttype.h>

/* float_type inheritable members */

jive_input *
jive_float_type_create_input_(const jive_type * self, struct jive_node * node, size_t index,
	jive_output * initial_operand);

jive_output *
jive_float_type_create_output_(const jive_type * self, struct jive_node * node, size_t index);

jive_gate *
jive_float_type_create_gate_(const jive_type * self, struct jive_graph * graph, const char * name);

jive_type *
jive_float_type_copy_(const jive_type * self);

#endif
