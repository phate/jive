#ifndef JIVE_VSDG_STATETYPE_H
#define JIVE_VSDG_STATETYPE_H

#include <jive/vsdg/basetype.h>

typedef struct jive_state_type jive_state_type;
typedef struct jive_state_input jive_state_input;
typedef struct jive_state_output jive_state_output;
typedef struct jive_state_output jive_state;
typedef struct jive_state_gate jive_state_gate;
typedef struct jive_state_resource jive_state_resource;

extern const jive_type_class JIVE_STATE_TYPE;
#define JIVE_DECLARE_STATE_TYPE(name) const jive_state_type name##_struct = {{&JIVE_STATE_TYPE}}; const jive_type * name = &name##_struct.base

struct jive_state_type {
	jive_type base;
};

extern const jive_input_class JIVE_STATE_INPUT;
struct jive_state_input {
	jive_input base;
};

extern const jive_output_class JIVE_STATE_OUTPUT;
struct jive_state_output {
	jive_output base;
};

extern const jive_gate_class JIVE_STATE_GATE;
struct jive_state_gate {
	jive_gate base;
};

#endif
