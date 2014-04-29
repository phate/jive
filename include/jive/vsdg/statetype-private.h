/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_STATETYPE_PRIVATE_H
#define JIVE_VSDG_STATETYPE_PRIVATE_H

#include <jive/vsdg/statetype.h>

/* state_type inheritable members */

void
jive_state_type_fini_(jive_type * self);

jive_input *
jive_state_type_create_input_(const jive_type * self, struct jive_node * node, size_t index, jive_output * initial_operand);

jive_output *
jive_state_type_create_output_(const jive_type * self, struct jive_node * node, size_t index);

jive_gate *
jive_state_type_create_gate_(const jive_type * self, struct jive_graph * graph, const char * name);

jive_type *
jive_state_type_copy_(const jive_type * self);

#endif
