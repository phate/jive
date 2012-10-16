/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_ANCHORTYPE_PRIVATE_H
#define JIVE_VSDG_ANCHORTYPE_PRIVATE_H

#include <jive/vsdg/anchortype.h>

/* anchor_type inheritable members */

jive_input *
jive_anchor_type_create_input_(const jive_type * self, struct jive_node * node, size_t index, jive_output * initial_operand);

jive_output *
jive_anchor_type_create_output_(const jive_type * self, struct jive_node * node, size_t index);

/* anchor_input inheritable members */

void
jive_anchor_input_init_(jive_anchor_input * self, struct jive_node * node, size_t index, jive_output * origin);

void
jive_anchor_input_fini_(jive_input * self_);

const jive_type *
jive_anchor_input_get_type_(const jive_input * self);

/* anchor_output inheritable members */

void
jive_anchor_output_init_(jive_anchor_output * self, struct jive_node * node, size_t index);

const jive_type *
jive_anchor_output_get_type_(const jive_output * self);

#endif
