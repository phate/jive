#ifndef JIVE_ARCH_STACKFRAME_H
#define JIVE_ARCH_STACKFRAME_H

#include <jive/vsdg/resource.h>

typedef struct jive_stackframe jive_stackframe;
typedef struct jive_stackframe_class jive_stackframe_class;

typedef struct jive_stackvar_type jive_stackvar_type;
typedef struct jive_stackvar_input jive_stackvar_input;
typedef struct jive_stackvar_output jive_stackvar_output;
typedef struct jive_stackvar_gate jive_stackvar_gate;

typedef struct jive_stackslot_size_class jive_stackslot_size_class;
typedef struct jive_fixed_stackslot_class jive_fixed_stackslot_class;
typedef struct jive_stackslot jive_stackslot;

extern const jive_stackslot_size_class jive_stackslot_class_8_8;
extern const jive_stackslot_size_class jive_stackslot_class_16_16;
extern const jive_stackslot_size_class jive_stackslot_class_32_32;
extern const jive_stackslot_size_class jive_stackslot_class_64_64;
extern const jive_stackslot_size_class jive_stackslot_class_128_128;

const jive_resource_class *
jive_stackslot_size_class_get(size_t size, size_t alignment);

const jive_resource_class *
jive_fixed_stackslot_class_get(size_t size, int offset);

/* resource classes and names*/

struct jive_stackslot_size_class {
	jive_resource_class base;
	size_t size, alignment;
};

struct jive_fixed_stackslot_class {
	jive_stackslot_size_class base;
	
	const jive_resource_name * slot;
};

struct jive_stackslot {
	jive_resource_name base;
	int offset;
};

#endif
