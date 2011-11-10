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
jive_value_type_copy_(const jive_type * self, struct jive_context * context);

/* value_input inheritable members */

void
jive_value_input_init_(jive_value_input * self, struct jive_node * node, size_t index, jive_output * origin);

const jive_type *
jive_value_input_get_type_(const jive_input * self);

/* value_output inheritable members */

void
jive_value_output_init_(jive_value_output * self, struct jive_node * node, size_t index);

const jive_type *
jive_value_output_get_type_(const jive_output * self);

/* value_gate inheritable members */

void
jive_value_gate_init_(jive_value_gate * self, struct jive_graph * graph, const char name[]);

const jive_type *
jive_value_gate_get_type_(const jive_gate * self);

#endif
