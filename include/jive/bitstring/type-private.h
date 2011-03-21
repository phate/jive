#ifndef JIVE_BITSTRING_TYPE_PRIVATE_H
#define JIVE_BITSTRING_TYPE_PRIVATE_H

#include <jive/bitstring/type.h>

/* bitstring_type inheritable members */

char *
_jive_bitstring_type_get_label(const jive_type * self);

jive_input *
_jive_bitstring_type_create_input(const jive_type * self, struct jive_node * node, size_t index, jive_output * initial_operand);

jive_output *
_jive_bitstring_type_create_output(const jive_type * self, struct jive_node * node, size_t index);

jive_gate *
_jive_bitstring_type_create_gate(const jive_type * self, struct jive_graph * graph, const char * name);

/* bitstring_input inheritable members */

void
_jive_bitstring_input_init(jive_bitstring_input * self, size_t nbits, struct jive_node * node, size_t index, jive_output * origin);

const jive_type *
_jive_bitstring_input_get_type(const jive_input * self);

/* bitstring_output inheritable members */

void
_jive_bitstring_output_init(jive_bitstring_output * self, size_t nbits, struct jive_node * node, size_t index);

const jive_type *
_jive_bitstring_output_get_type(const jive_output * self);

/* bitstring_gate inheritable members */

void
_jive_bitstring_gate_init(jive_bitstring_gate * self, size_t nbits, struct jive_graph * graph, const char name[]);

const jive_type *
_jive_bitstring_gate_get_type(const jive_gate * self);

#endif
