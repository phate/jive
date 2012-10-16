/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
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
jive_type_copy_(const jive_type * self, struct jive_context * ctx);

char *
jive_type_get_label_(const jive_type * self);

jive_input *
jive_type_create_input_(const jive_type * self, struct jive_node * node, size_t index, jive_output * initial_operand);

jive_output *
jive_type_create_output_(const jive_type * self, struct jive_node * node, size_t index);

jive_gate *
jive_type_create_gate_(const jive_type * self, struct jive_graph * graph, const char * name);

bool
jive_type_equals_(const jive_type * self, const jive_type * other);

/* inheritable input member functions */

void
jive_input_init_(jive_input * self, struct jive_node * node, size_t index, jive_output * origin);

void
jive_input_fini_(jive_input * self);

char *
jive_input_get_label_(const jive_input * self);

const jive_type *
jive_input_get_type_(const jive_input * self);

/* inheritable output member functions */

void
jive_output_init_(
	jive_output * self,
	struct jive_node * node,
	size_t index);

void
jive_output_fini_(jive_output * self);

char *
jive_output_get_label_(const jive_output * self);

const jive_type *
jive_output_get_type_(const jive_output * self);


/* inheritable gate member functions */

void
jive_gate_init_(jive_gate * self, struct jive_graph * graph, const char name[]);

void
jive_gate_fini_(jive_gate * self);

char *
jive_gate_get_label_(const jive_gate * self);

const jive_type *
jive_gate_get_type_(const jive_gate * self);

#endif
