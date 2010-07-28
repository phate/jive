#ifndef JIVE_VSDG_STATETYPE_PRIVATE_H
#define JIVE_VSDG_STATETYPE_PRIVATE_H

#include <jive/vsdg/statetype.h>

/* state_type inheritable members */

jive_input *
_jive_state_type_create_input(const jive_type * self, struct jive_node * node, size_t index, jive_output * initial_operand);

jive_output *
_jive_state_type_create_output(const jive_type * self, struct jive_node * node, size_t index);

jive_resource *
_jive_state_type_create_resource(const jive_type * self, struct jive_graph * graph);

jive_gate *
_jive_state_type_create_gate(const jive_type * self, struct jive_graph * graph, const char * name);

/* state_input inheritable members */

void
_jive_state_input_init(jive_state_input * self, struct jive_node * node, size_t index, jive_output * origin);

const jive_type *
_jive_state_input_get_type(const jive_input * self);

/* state_output inheritable members */

void
_jive_state_output_init(jive_state_output * self, struct jive_node * node, size_t index);

const jive_type *
_jive_state_output_get_type(const jive_output * self);
	
/* state_resource inheritable members */

void
_jive_state_resource_init(jive_state_resource * self);

const jive_type *
_jive_state_resource_get_type(const jive_resource * self);

/* state_gate inheritable members */

void
_jive_state_gate_init(jive_state_gate * self, struct jive_graph * graph, const char name[]);

char *
_jive_state_gate_get_label(const jive_gate * self);

const jive_type *
_jive_state_gate_get_type(const jive_gate * self);

#endif
