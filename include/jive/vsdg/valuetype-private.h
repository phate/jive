#ifndef JIVE_VSDG_VALUETYPE_PRIVATE_H
#define JIVE_VSDG_VALUETYPE_PRIVATE_H

#include <jive/vsdg/valuetype.h>

/* value_type inheritable members */

jive_input *
_jive_value_type_create_input(const jive_type * self, struct jive_node * node, size_t index, jive_output * initial_operand);

jive_output *
_jive_value_type_create_output(const jive_type * self, struct jive_node * node, size_t index);

jive_gate *
_jive_value_type_create_gate(const jive_type * self, struct jive_graph * graph, const char * name);

/* value_input inheritable members */

void
_jive_value_input_init(jive_value_input * self, struct jive_node * node, size_t index, jive_output * origin);

const jive_type *
_jive_value_input_get_type(const jive_input * self);

/* value_output inheritable members */

void
_jive_value_output_init(jive_value_output * self, struct jive_node * node, size_t index);

const jive_type *
_jive_value_output_get_type(const jive_output * self);

/* value_gate inheritable members */

void
_jive_value_gate_init(jive_value_gate * self, struct jive_graph * graph, const char name[]);

const jive_type *
_jive_value_gate_get_type(const jive_gate * self);

#endif
