#ifndef JIVE_ARCH_STACKFRAME_PRIVATE_H
#define JIVE_ARCH_STACKFRAME_PRIVATE_H

#include <jive/arch/stackframe.h>
#include <jive/vsdg/region.h>
#include <jive/debug-private.h>

static inline void
_jive_stackframe_init(jive_stackframe * self, jive_region * region, struct jive_output * stackptr)
{
	DEBUG_ASSERT(region->stackframe == 0);
	self->region = region;
	self->stackptr = stackptr;
	self->slots.first = self->slots.last = 0;
	region->stackframe = self;
}

void
_jive_stackframe_fini(jive_stackframe * self);

/* stackslots */

/* type inheritable members */

char *
_jive_stackslot_type_get_label(const jive_type * self);

jive_input *
_jive_stackslot_type_create_input(const jive_type * self, struct jive_node * node, size_t index, jive_output * initial_operand);

jive_output *
_jive_stackslot_type_create_output(const jive_type * self, struct jive_node * node, size_t index);

jive_resource *
_jive_stackslot_type_create_resource(const jive_type * self, struct jive_graph * graph);

jive_gate *
_jive_stackslot_type_create_gate(const jive_type * self, struct jive_graph * graph, const char * name);

bool
_jive_stackslot_type_equals(const jive_type * self, const jive_type * other);

bool
_jive_stackslot_type_accepts(const jive_type * self, const jive_type * other);

/* input inheritable members */

char *
_jive_stackslot_input_get_label(const jive_input * self);

void
_jive_stackslot_input_init(jive_stackslot_input * self, const struct jive_regcls * regcls, struct jive_node * node, size_t index, jive_output * origin);

const jive_type *
_jive_stackslot_input_get_type(const jive_input * self);

/* output inheritable members */

char *
_jive_stackslot_output_get_label(const jive_output * self);

void
_jive_stackslot_output_init(jive_stackslot_output * self, const struct jive_regcls * regcls, struct jive_node * node, size_t index);

const jive_type *
_jive_stackslot_output_get_type(const jive_output * self);
	
/* resource inheritable members */

char *
_jive_stackslot_resource_get_label(const jive_resource * self);

void
_jive_stackslot_resource_init(jive_stackslot_resource * self, const struct jive_regcls * regcls, struct jive_graph * graph);

void
_jive_stackslot_resource_fini(jive_resource * self);

const jive_type *
_jive_stackslot_resource_get_type(const jive_resource * self);

/* gate inheritable members */

char *
_jive_stackslot_gate_get_label(const jive_gate * self);

void
_jive_stackslot_gate_init(jive_stackslot_gate * self, const struct jive_regcls * regcls, struct jive_graph * graph, const char name[]);

char *
_jive_stackslot_gate_get_label(const jive_gate * self);

const jive_type *
_jive_stackslot_gate_get_type(const jive_gate * self);



#endif
