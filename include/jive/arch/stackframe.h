#ifndef JIVE_ARCH_STACKFRAME_H
#define JIVE_ARCH_STACKFRAME_H

#include <jive/vsdg/statetype.h>

typedef struct jive_stackframe jive_stackframe;
typedef struct jive_stackframe_class jive_stackframe_class;

typedef struct jive_stackslot_type jive_stackslot_type;
typedef struct jive_stackslot_input jive_stackslot_input;
typedef struct jive_stackslot_output jive_stackslot_output;
typedef struct jive_stackslot_output jive_stackslot;
typedef struct jive_stackslot_gate jive_stackslot_gate;
typedef struct jive_stackslot_resource jive_stackslot_resource;

struct jive_output;
struct jive_region;
struct jive_graph;

struct jive_stackframe {
	const jive_stackframe_class * class_;
	
	struct {
		jive_stackslot_resource * first;
		jive_stackslot_resource * last;
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

struct jive_stackslot_type {
	jive_state_type base;
	const struct jive_regcls * regcls;
};

extern const jive_type_class JIVE_STACKSLOT_TYPE;
#define JIVE_DECLARE_STACKSLOT_TYPE(name, regcls) const jive_stackslot_type name##_struct = jive_stackslot_type_create(regcls); const jive_type * name = &name##_struct.base.base

extern const jive_input_class JIVE_STACKSLOT_INPUT;
struct jive_stackslot_input {
	jive_state_input base;
	jive_stackslot_type type;
};

extern const jive_output_class JIVE_STACKSLOT_OUTPUT;
struct jive_stackslot_output {
	jive_state_output base;
	jive_stackslot_type type;
};

extern const jive_gate_class JIVE_STACKSLOT_GATE;
struct jive_stackslot_gate {
	jive_state_gate base;
	jive_stackslot_type type;
};

extern const jive_resource_class JIVE_STACKSLOT_RESOURCE;
struct jive_stackslot_resource {
	jive_state_resource base;
	jive_stackslot_type type;
	jive_stackframe * stackframe;
	struct {
		jive_stackslot_resource * prev;
		jive_stackslot_resource * next;
	} stackframe_slots_list;
	long offset;
};

static inline jive_stackslot_type
jive_stackslot_type_create(const struct jive_regcls * regcls)
{
	jive_stackslot_type type;
	type.base.base.class_ = &JIVE_STACKSLOT_TYPE;
	type.regcls = regcls;
	return type;
}

static inline jive_stackslot_resource *
jive_stackslot_resource_cast(jive_resource * self)
{
	if (self && jive_resource_isinstance(self, &JIVE_STACKSLOT_RESOURCE))
		return (jive_stackslot_resource *) self;
	else
		return 0;
}

void
jive_stackframe_destroy(jive_stackframe * self);

#endif
