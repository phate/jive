/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_MEMORYTYPE_H
#define JIVE_ARCH_MEMORYTYPE_H

#include <jive/vsdg/statetype.h>

typedef struct jive_memory_type jive_memory_type;
typedef struct jive_memory_input jive_memory_input;
typedef struct jive_memory_output jive_memory_output;
typedef struct jive_memory_output jive_memory;
typedef struct jive_memory_gate jive_memory_gate;
typedef struct jive_memory_resource jive_memory_resource;

extern const jive_type_class JIVE_MEMORY_TYPE;
#define JIVE_DECLARE_MEMORY_TYPE(name) \
	jive_memory_type name##_struct; name##_struct.class_ = &JIVE_MEMORY_TYPE; \
	const jive_type * name = &name##_struct

struct jive_memory_type : public jive_state_type {
	jive_memory_type() { class_ = &JIVE_MEMORY_TYPE; }
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
