#ifndef JIVE_VSDG_ANCHORTYPE_PRIVATE_H
#define JIVE_VSDG_ANCHORTYPE_PRIVATE_H

#include <jive/vsdg/anchortype.h>

/* anchor_type inheritable members */

jive_input *
_jive_anchor_type_create_input(const jive_type * self, struct jive_node * node, size_t index, jive_output * initial_operand);

jive_output *
_jive_anchor_type_create_output(const jive_type * self, struct jive_node * node, size_t index);

/* anchor_input inheritable members */

void
_jive_anchor_input_init(jive_anchor_input * self, struct jive_node * node, size_t index, jive_output * origin);

void
_jive_anchor_input_fini(jive_input * self_);

const jive_type *
_jive_anchor_input_get_type(const jive_input * self);

/* anchor_output inheritable members */

void
_jive_anchor_output_init(jive_anchor_output * self, struct jive_node * node, size_t index);

const jive_type *
_jive_anchor_output_get_type(const jive_output * self);

#endif
