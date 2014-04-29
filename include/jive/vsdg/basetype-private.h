/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_BASETYPE_PRIVATE_H
#define JIVE_VSDG_BASETYPE_PRIVATE_H

#include <jive/vsdg/basetype.h>

void
jive_input_internal_divert_origin(jive_input * self, jive_output * new_origin);

/* inheritable type member functions */

void
jive_type_fini_(jive_type * self);

jive_type *
jive_type_copy_(const jive_type * self);

void
jive_type_get_label_(const jive_type * self, struct jive_buffer * buffer);

jive_input *
jive_type_create_input_(const jive_type * self, struct jive_node * node, size_t index, jive_output * initial_operand);

jive_output *
jive_type_create_output_(const jive_type * self, struct jive_node * node, size_t index);

jive_gate *
jive_type_create_gate_(const jive_type * self, struct jive_graph * graph, const char * name);

bool
jive_type_equals_(const jive_type * self, const jive_type * other);

#endif
