/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_VALUETYPE_PRIVATE_H
#define JIVE_VSDG_VALUETYPE_PRIVATE_H

#include <jive/vsdg/valuetype.h>

/* value_type inheritable members */

void
jive_value_type_fini_(jive_type * self);

jive_input *
jive_value_type_create_input_(const jive_type * self, struct jive_node * node, size_t index, jive_output * initial_operand);

jive_output *
jive_value_type_create_output_(const jive_type * self, struct jive_node * node, size_t index);

jive_gate *
jive_value_type_create_gate_(const jive_type * self, struct jive_graph * graph, const char * name);

jive_type *
jive_value_type_copy_(const jive_type * self);

/* value_input inheritable members */

const jive_type *
jive_value_input_get_type_(const jive_input * self);

/* value_output inheritable members */

const jive_type *
jive_value_output_get_type_(const jive_output * self);

/* value_gate inheritable members */

const jive_type *
jive_value_gate_get_type_(const jive_gate * self);

#endif
