#ifndef JIVE_TYPES_BITSTRING_TYPE_PRIVATE_H
#define JIVE_TYPES_BITSTRING_TYPE_PRIVATE_H

#include <jive/types/bitstring/type.h>

/* bitstring_type inheritable members */

void
jive_bitstring_type_fini_(jive_type * self);

char *
jive_bitstring_type_get_label_(const jive_type * self);

jive_input *
jive_bitstring_type_create_input_(const jive_type * self, struct jive_node * node, size_t index, jive_output * initial_operand);

jive_output *
jive_bitstring_type_create_output_(const jive_type * self, struct jive_node * node, size_t index);

jive_gate *
jive_bitstring_type_create_gate_(const jive_type * self, struct jive_graph * graph, const char * name);

jive_type *
jive_bitstring_type_copy_(const jive_type * self, struct jive_context * context);

bool
jive_bitstring_type_equals_(const jive_type * self, const jive_type * other);

/* bitstring_input inheritable members */

void
jive_bitstring_input_init_(jive_bitstring_input * self, size_t nbits, struct jive_node * node, size_t index, jive_output * origin);

const jive_type *
jive_bitstring_input_get_type_(const jive_input * self);

/* bitstring_output inheritable members */

void
jive_bitstring_output_init_(jive_bitstring_output * self, size_t nbits, struct jive_node * node, size_t index);

const jive_type *
jive_bitstring_output_get_type_(const jive_output * self);

/* bitstring_gate inheritable members */

void
jive_bitstring_gate_init_(jive_bitstring_gate * self, size_t nbits, struct jive_graph * graph, const char name[]);

const jive_type *
jive_bitstring_gate_get_type_(const jive_gate * self);

#endif
