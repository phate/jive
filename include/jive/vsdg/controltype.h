#ifndef JIVE_VSDG_CONTROLTYPE_H
#define JIVE_VSDG_CONTROLTYPE_H

#include <jive/vsdg/basetype.h>

typedef struct jive_control_type jive_control_type;
typedef struct jive_control_input jive_control_input;
typedef struct jive_control_output jive_control_output;

extern const jive_type_class JIVE_CONTROL_TYPE;
#define JIVE_DECLARE_CONTROL_TYPE(name) const jive_control_type name##_struct = {{&JIVE_CONTROL_TYPE}}; const jive_type * name = &name##_struct.base

struct jive_control_type {
	jive_type base;
};

extern const jive_input_class JIVE_CONTROL_INPUT;
struct jive_control_input {
	jive_input base;
};

extern const jive_output_class JIVE_CONTROL_OUTPUT;
struct jive_control_output {
	jive_output base;
};

#endif
