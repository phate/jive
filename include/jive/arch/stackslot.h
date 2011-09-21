#ifndef JIVE_ARCH_STACKSLOT_H
#define JIVE_ARCH_STACKSLOT_H

#include <unistd.h>
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
typedef struct jive_callslot_class jive_callslot_class;
typedef struct jive_callslot jive_callslot;

extern const jive_resource_class jive_root_stackslot_class;

extern const jive_stackslot_size_class jive_stackslot_class_1_1;
extern const jive_stackslot_size_class jive_stackslot_class_2_2;
extern const jive_stackslot_size_class jive_stackslot_class_4_4;
extern const jive_stackslot_size_class jive_stackslot_class_8_8;
extern const jive_stackslot_size_class jive_stackslot_class_16_16;

const jive_resource_class *
jive_stackslot_size_class_get(size_t size, size_t alignment);

const jive_resource_class *
jive_fixed_stackslot_class_get(size_t size, size_t alignment, ssize_t offset);

const jive_resource_class *
jive_callslot_class_get(size_t size, size_t alignment, ssize_t offset);

const jive_resource_name *
jive_stackslot_name_get(size_t size, size_t alignment, ssize_t offset);

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

struct jive_callslot_class {
	jive_stackslot_size_class base;
	int offset;
	
	const jive_resource_name * slot;
};

struct jive_callslot {
	jive_resource_name base;
	int offset;
};

extern const jive_resource_class_class JIVE_STACK_RESOURCE;
extern const jive_resource_class_class JIVE_STACK_FRAMESLOT_RESOURCE;
extern const jive_resource_class_class JIVE_STACK_CALLSLOT_RESOURCE;

#endif
