#ifndef JIVE_VSDG_CONTROLTYPE_PRIVATE_H
#define JIVE_VSDG_CONTROLTYPE_PRIVATE_H

#include <jive/vsdg/controltype.h>

/* control_type inheritable members */

jive_input *
_jive_control_type_create_input(const jive_type * self, struct jive_node * node, size_t index, jive_output * initial_operand);

jive_output *
_jive_control_type_create_output(const jive_type * self, struct jive_node * node, size_t index);

/* control_input inheritable members */

void
_jive_control_input_init(jive_control_input * self, struct jive_node * node, size_t index, jive_output * origin);

void
_jive_control_input_fini(jive_input * self_);

const jive_type *
_jive_control_input_get_type(const jive_input * self);

/* control_output inheritable members */

void
_jive_control_output_init(jive_control_output * self, struct jive_node * node, size_t index);

const jive_type *
_jive_control_output_get_type(const jive_output * self);

#endif
