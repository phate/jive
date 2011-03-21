#ifndef JIVE_VSDG_BASETYPE_PRIVATE_H
#define JIVE_VSDG_BASETYPE_PRIVATE_H

#include <jive/vsdg/basetype.h>

void
jive_input_internal_divert_origin(jive_input * self, jive_output * new_origin);

/* inheritable type member functions */

void
_jive_type_fini(jive_type * self);

jive_type *
_jive_type_copy(const jive_type * self, struct jive_context * ctx);

char *
_jive_type_get_label(const jive_type * self);

jive_input *
_jive_type_create_input(const jive_type * self, struct jive_node * node, size_t index, jive_output * initial_operand);

jive_output *
_jive_type_create_output(const jive_type * self, struct jive_node * node, size_t index);

jive_gate *
_jive_type_create_gate(const jive_type * self, struct jive_graph * graph, const char * name);

bool
_jive_type_equals(const jive_type * self, const jive_type * other);

bool
_jive_type_accepts(const jive_type * self, const jive_type * other);

/* inheritable input member functions */

void
_jive_input_init(jive_input * self, struct jive_node * node, size_t index, jive_output * origin);

void
_jive_input_fini(jive_input * self);

char *
_jive_input_get_label(const jive_input * self);

const jive_type *
_jive_input_get_type(const jive_input * self);

/* inheritable output member functions */

void
_jive_output_init(
	jive_output * self,
	struct jive_node * node,
	size_t index);

void
_jive_output_fini(jive_output * self);

char *
_jive_output_get_label(const jive_output * self);

const jive_type *
_jive_output_get_type(const jive_output * self);


/* inheritable gate member functions */

void
_jive_gate_init(jive_gate * self, struct jive_graph * graph, const char name[]);

void
_jive_gate_fini(jive_gate * self);

char *
_jive_gate_get_label(const jive_gate * self);

const jive_type *
_jive_gate_get_type(const jive_gate * self);

#endif
