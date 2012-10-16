/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_CONTROLTYPE_H
#define JIVE_VSDG_CONTROLTYPE_H

#include <jive/vsdg/statetype.h>

typedef struct jive_control_type jive_control_type;
typedef struct jive_control_input jive_control_input;
typedef struct jive_control_output jive_control_output;
typedef struct jive_control_gate jive_control_gate;

extern const jive_type_class JIVE_CONTROL_TYPE;
#define JIVE_DECLARE_CONTROL_TYPE(name) const jive_control_type name##_struct = {{{&JIVE_CONTROL_TYPE}}}; const jive_type * name = &name##_struct.base.base

struct jive_control_type {
	jive_state_type base;
};

extern const jive_input_class JIVE_CONTROL_INPUT;
struct jive_control_input {
	jive_state_input base;
};

extern const jive_output_class JIVE_CONTROL_OUTPUT;
struct jive_control_output {
	jive_state_output base;
	bool active;
};

extern const jive_gate_class JIVE_CONTROL_GATE;
struct jive_control_gate {
	jive_state_gate base;
};


#endif
