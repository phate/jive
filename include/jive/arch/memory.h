#ifndef JIVE_ARCH_MEMORY_H
#define JIVE_ARCH_MEMORY_H

#include <jive/vsdg/statetype.h>

typedef struct jive_memory_type jive_memory_type;
typedef struct jive_memory_input jive_memory_input;
typedef struct jive_memory_output jive_memory_output;
typedef struct jive_memory_output jive_memory;
typedef struct jive_memory_gate jive_memory_gate;
typedef struct jive_memory_resource jive_memory_resource;

extern const jive_type_class JIVE_MEMORY_TYPE;
#define JIVE_DECLARE_MEMORY_TYPE(name) const jive_memory_type name##_struct = {{{&JIVE_MEMORY_TYPE}}}; const jive_type * name = &name##_struct.base.base

struct jive_memory_type {
	jive_state_type base;
};

extern const jive_input_class JIVE_MEMORY_INPUT;
struct jive_memory_input {
	jive_state_input base;
};

extern const jive_output_class JIVE_MEMORY_OUTPUT;
struct jive_memory_output {
	jive_state_output base;
};

extern const jive_gate_class JIVE_MEMORY_GATE;
struct jive_memory_gate {
	jive_state_gate base;
};

#endif
