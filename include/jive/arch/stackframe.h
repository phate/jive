#ifndef JIVE_ARCH_STACKFRAME_H
#define JIVE_ARCH_STACKFRAME_H

#include <jive/vsdg/statetype.h>

typedef struct jive_stackframe jive_stackframe;
typedef struct jive_stackframe_class jive_stackframe_class;

typedef struct jive_stackvar_type jive_stackvar_type;
typedef struct jive_stackvar_input jive_stackvar_input;
typedef struct jive_stackvar_output jive_stackvar_output;
typedef struct jive_stackvar_output jive_stackvar;
typedef struct jive_stackvar_gate jive_stackvar_gate;
typedef struct jive_stackvar_resource jive_stackvar_resource;
typedef struct jive_stackslot jive_stackslot;

struct jive_output;
struct jive_region;
struct jive_graph;

struct jive_stackframe {
	const jive_stackframe_class * class_;
	
	struct {
		jive_stackvar_resource * first;
		jive_stackvar_resource * last;
	} vars;
	struct {
		jive_stackslot * first;
		jive_stackslot * last;
	} slots;
	
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
	const struct jive_regcls * regcls;
};

extern const jive_type_class JIVE_STACKSLOT_TYPE;
#define JIVE_DECLARE_STACKSLOT_TYPE(name, regcls) const jive_stackvar_type name##_struct = jive_stackvar_type_create(regcls); const jive_type * name = &name##_struct.base.base

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

extern const jive_resource_class JIVE_STACKSLOT_RESOURCE;
struct jive_stackvar_resource {
	jive_state_resource base;
	jive_stackvar_type type;
	jive_stackframe * stackframe;
	struct {
		jive_stackvar_resource * prev;
		jive_stackvar_resource * next;
	} stackframe_vars_list;
	jive_stackslot * slot;
};

struct jive_stackslot {
	struct {
		jive_stackslot * prev;
		jive_stackslot * next;
	} stackframe_slots_list;
	long offset;
	jive_stackframe * stackframe;
};

static inline jive_stackvar_type
jive_stackvar_type_create(const struct jive_regcls * regcls)
{
	jive_stackvar_type type;
	type.base.base.class_ = &JIVE_STACKSLOT_TYPE;
	type.regcls = regcls;
	return type;
}

static inline jive_stackvar_resource *
jive_stackvar_resource_cast(jive_resource * self)
{
	if (jive_resource_isinstance(self, &JIVE_STACKSLOT_RESOURCE))
		return (jive_stackvar_resource *) self;
	else
		return 0;
}

jive_stackslot *
jive_stackslot_create(jive_stackframe * stackframe, long offset);

void
jive_stackframe_destroy(jive_stackframe * self);

#endif
