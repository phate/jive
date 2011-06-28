#ifndef JIVE_ARCH_STACKFRAME_H
#define JIVE_ARCH_STACKFRAME_H

#include <jive/vsdg/resource.h>
#include <jive/vsdg/statetype.h>

typedef struct jive_stackframe jive_stackframe;
typedef struct jive_stackframe_class jive_stackframe_class;

typedef struct jive_stackvar_type jive_stackvar_type;
typedef struct jive_stackvar_input jive_stackvar_input;
typedef struct jive_stackvar_output jive_stackvar_output;
typedef struct jive_stackvar_gate jive_stackvar_gate;

typedef struct jive_stackslot_size_class jive_stackslot_size_class;
typedef struct jive_reserved_stackslot_class jive_reserved_stackslot_class;
typedef struct jive_stackslot jive_stackslot;

struct jive_output;
struct jive_region;
struct jive_graph;

extern const jive_stackslot_size_class jive_stackslot_class_8_8;
extern const jive_stackslot_size_class jive_stackslot_class_16_16;
extern const jive_stackslot_size_class jive_stackslot_class_32_32;
extern const jive_stackslot_size_class jive_stackslot_class_64_64;
extern const jive_stackslot_size_class jive_stackslot_class_128_128;

extern const jive_reserved_stackslot_class jive_reserved_stackslot_class_32_0;
extern const jive_reserved_stackslot_class jive_reserved_stackslot_class_32_4;
extern const jive_reserved_stackslot_class jive_reserved_stackslot_class_32_8;
extern const jive_reserved_stackslot_class jive_reserved_stackslot_class_32_12;
extern const jive_reserved_stackslot_class jive_reserved_stackslot_class_32_16;
extern const jive_reserved_stackslot_class jive_reserved_stackslot_class_32_20;
extern const jive_reserved_stackslot_class jive_reserved_stackslot_class_32_24;
extern const jive_reserved_stackslot_class jive_reserved_stackslot_class_32_28;

struct jive_stackframe {
	const jive_stackframe_class * class_;
	
	struct jive_context * context;
	
	struct {
		jive_stackslot * first;
		jive_stackslot * last;
	} slots;
	
	struct {
		jive_stackslot_size_class * first;
		jive_stackslot_size_class * last;
	} stackslot_size_classes;
	
	struct {
		jive_reserved_stackslot_class * first;
		jive_reserved_stackslot_class * last;
	} reserved_stackslot_classes;
	
	struct jive_region * region;
	struct jive_output * stackptr;
};

struct jive_stackframe_class {
	const jive_stackframe_class * parent;
	void (*fini)(jive_stackframe * self);
	void (*layout)(jive_stackframe * self);
};

extern const jive_stackframe_class JIVE_STACKFRAME_CLASS;

struct jive_stackvar_type {
	jive_state_type base;
	size_t size;
};

extern const jive_type_class JIVE_STACKVAR_TYPE;
#define JIVE_DECLARE_STACKVAR_TYPE(name, regcls) const jive_stackvar_type name##_struct = jive_stackvar_type_create(regcls); const jive_type * name = &name##_struct.base.base

extern const jive_input_class JIVE_STACKSLOT_INPUT;
struct jive_stackvar_input {
	jive_state_input base;
	jive_stackvar_type type;
	jive_stackslot * required_slot;
};

extern const jive_output_class JIVE_STACKSLOT_OUTPUT;
struct jive_stackvar_output {
	jive_state_output base;
	jive_stackvar_type type;
	jive_stackslot * required_slot;
};

extern const jive_gate_class JIVE_STACKSLOT_GATE;
struct jive_stackvar_gate {
	jive_state_gate base;
	jive_stackvar_type type;
};

void
jive_stackframe_destroy(jive_stackframe * self);

const struct jive_resource_class *
jive_stackframe_get_stackslot_resource_class(jive_stackframe * self, size_t size);

const struct jive_resource_class *
jive_stackframe_get_reserved_stackslot_resource_class(jive_stackframe * self, size_t size, long offset);

static inline void
jive_stackframe_layout(jive_stackframe * self)
{
	self->class_->layout(self);
}

/* resource classes */

struct jive_stackslot_size_class {
	jive_resource_class base;
	size_t size;
	
	struct {
		jive_stackslot_size_class * prev;
		jive_stackslot_size_class * next;
	} stackframe_stackslot_size_class_list;
	
	jive_stackframe * stackframe;
};

struct jive_reserved_stackslot_class {
	jive_stackslot_size_class base;
	
	struct {
		jive_reserved_stackslot_class * prev;
		jive_reserved_stackslot_class * next;
	} stackframe_reserved_stackslot_class_list;
	
	const jive_resource_name * slot;
};

/* resource names */

struct jive_stackslot {
	jive_resource_name base;
	struct {
		jive_stackslot * prev;
		jive_stackslot * next;
	} stackframe_slots_list;
	long offset;
	jive_stackframe * stackframe;
};

jive_stackslot *
jive_stackslot_create(const jive_resource_class * rescls, long offset);


#endif
