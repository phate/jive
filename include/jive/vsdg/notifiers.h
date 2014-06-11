/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_NOTIFIERS_H
#define JIVE_VSDG_NOTIFIERS_H

#include <jive/common.h>
#include <jive/context.h>

namespace jive {
	class gate;
	class input;
	class output;
}

struct jive_region;
struct jive_node;
struct jive_variable;
struct jive_ssavar;
struct jive_resource_class;
struct jive_resource_name;

typedef struct jive_notifier jive_notifier;
typedef struct jive_notifier_class jive_notifier_class;

struct jive_notifier_class {
	void (*disconnect)(jive_notifier * self);
};

struct jive_notifier {
	const jive_notifier_class * class_;
	struct jive_context * context;
};

JIVE_EXPORTED_INLINE void
jive_notifier_disconnect(jive_notifier * self)
{
	self->class_->disconnect(self);
}

/* node notifiers */

typedef void (*jive_node_notifier_function)(void * closure, struct jive_node * node);
typedef struct jive_node_notifier jive_node_notifier;
typedef struct jive_node_notifier_slot jive_node_notifier_slot;

struct jive_node_notifier_slot {
	struct {
		jive_node_notifier * first;
		jive_node_notifier * last;
	} notifiers;
	struct jive_context * context;
};

JIVE_EXPORTED_INLINE void
jive_node_notifier_slot_init(jive_node_notifier_slot * self, jive_context * context)
{
	self->notifiers.first = self->notifiers.last = 0;
	self->context = context;
}

void
jive_node_notifier_slot_fini(jive_node_notifier_slot * self);

jive_notifier *
jive_node_notifier_slot_connect(jive_node_notifier_slot * self, jive_node_notifier_function function, void * closure);

void
jive_node_notifier_slot_call(const jive_node_notifier_slot * self, struct jive_node * node);

/* node depth notifiers */

typedef void (*jive_node_depth_notifier_function)(void * closure, struct jive_node * node, size_t old_depth);
typedef struct jive_node_depth_notifier jive_node_depth_notifier;
typedef struct jive_node_depth_notifier_slot jive_node_depth_notifier_slot;

struct jive_node_depth_notifier_slot {
	struct {
		jive_node_depth_notifier * first;
		jive_node_depth_notifier * last;
	} notifiers;
	struct jive_context * context;
};

JIVE_EXPORTED_INLINE void
jive_node_depth_notifier_slot_init(jive_node_depth_notifier_slot * self, jive_context * context)
{
	self->notifiers.first = self->notifiers.last = 0;
	self->context = context;
}

void
jive_node_depth_notifier_slot_fini(jive_node_depth_notifier_slot * self);

jive_notifier *
jive_node_depth_notifier_slot_connect(jive_node_depth_notifier_slot * self, jive_node_depth_notifier_function function, void * closure);

void
jive_node_depth_notifier_slot_call(const jive_node_depth_notifier_slot * self, struct jive_node * node, size_t old_depth);

/* input notifiers */

typedef void (*jive_input_notifier_function)(void * closure, jive::input * input);
typedef struct jive_input_notifier jive_input_notifier;
typedef struct jive_input_notifier_slot jive_input_notifier_slot;

struct jive_input_notifier_slot {
	struct {
		jive_input_notifier * first;
		jive_input_notifier * last;
	} notifiers;
	struct jive_context * context;
};

JIVE_EXPORTED_INLINE void
jive_input_notifier_slot_init(jive_input_notifier_slot * self, jive_context * context)
{
	self->notifiers.first = self->notifiers.last = 0;
	self->context = context;
}

void
jive_input_notifier_slot_fini(jive_input_notifier_slot * self);

jive_notifier *
jive_input_notifier_slot_connect(jive_input_notifier_slot * self, jive_input_notifier_function function, void * closure);

void
jive_input_notifier_slot_call(const jive_input_notifier_slot * self, jive::input * input);

/* input_change notifiers */

typedef void (*jive_input_change_notifier_function)(void * closure, jive::input * input,
	jive::output * old_origin, jive::output * new_origin);
typedef struct jive_input_change_notifier jive_input_change_notifier;
typedef struct jive_input_change_notifier_slot jive_input_change_notifier_slot;

struct jive_input_change_notifier_slot {
	struct {
		jive_input_change_notifier * first;
		jive_input_change_notifier * last;
	} notifiers;
	struct jive_context * context;
};

JIVE_EXPORTED_INLINE void
jive_input_change_notifier_slot_init(jive_input_change_notifier_slot * self, jive_context * context)
{
	self->notifiers.first = self->notifiers.last = 0;
	self->context = context;
}

void
jive_input_change_notifier_slot_fini(jive_input_change_notifier_slot * self);

jive_notifier *
jive_input_change_notifier_slot_connect(jive_input_change_notifier_slot * self, jive_input_change_notifier_function function, void * closure);

void
jive_input_change_notifier_slot_call(const jive_input_change_notifier_slot * self,
	jive::input * input, jive::output * old_origin, jive::output * new_origin);

/* output notifiers */

typedef void (*jive_output_notifier_function)(void * closure, jive::output * output);
typedef struct jive_output_notifier jive_output_notifier;
typedef struct jive_output_notifier_slot jive_output_notifier_slot;

struct jive_output_notifier_slot {
	struct {
		jive_output_notifier * first;
		jive_output_notifier * last;
	} notifiers;
	struct jive_context * context;
};

JIVE_EXPORTED_INLINE void
jive_output_notifier_slot_init(jive_output_notifier_slot * self, jive_context * context)
{
	self->notifiers.first = self->notifiers.last = 0;
	self->context = context;
}

void
jive_output_notifier_slot_fini(jive_output_notifier_slot * self);

jive_notifier *
jive_output_notifier_slot_connect(jive_output_notifier_slot * self, jive_output_notifier_function function, void * closure);

void
jive_output_notifier_slot_call(const jive_output_notifier_slot * self, jive::output * output);

/* variable notifiers */

typedef void (*jive_variable_notifier_function)(void * closure, struct jive_variable * variable);
typedef struct jive_variable_notifier jive_variable_notifier;
typedef struct jive_variable_notifier_slot jive_variable_notifier_slot;

struct jive_variable_notifier_slot {
	struct {
		jive_variable_notifier * first;
		jive_variable_notifier * last;
	} notifiers;
	struct jive_context * context;
};

JIVE_EXPORTED_INLINE void
jive_variable_notifier_slot_init(jive_variable_notifier_slot * self, jive_context * context)
{
	self->notifiers.first = self->notifiers.last = 0;
	self->context = context;
}

void
jive_variable_notifier_slot_fini(jive_variable_notifier_slot * self);

jive_notifier *
jive_variable_notifier_slot_connect(jive_variable_notifier_slot * self, jive_variable_notifier_function function, void * closure);

void
jive_variable_notifier_slot_call(const jive_variable_notifier_slot * self, struct jive_variable * variable);

/* ssavar notifiers */

typedef void (*jive_ssavar_notifier_function)(void * closure, struct jive_ssavar * ssavar);
typedef struct jive_ssavar_notifier jive_ssavar_notifier;
typedef struct jive_ssavar_notifier_slot jive_ssavar_notifier_slot;

struct jive_ssavar_notifier_slot {
	struct {
		jive_ssavar_notifier * first;
		jive_ssavar_notifier * last;
	} notifiers;
	struct jive_context * context;
};

JIVE_EXPORTED_INLINE void
jive_ssavar_notifier_slot_init(jive_ssavar_notifier_slot * self, jive_context * context)
{
	self->notifiers.first = self->notifiers.last = 0;
	self->context = context;
}

void
jive_ssavar_notifier_slot_fini(jive_ssavar_notifier_slot * self);

jive_notifier *
jive_ssavar_notifier_slot_connect(jive_ssavar_notifier_slot * self, jive_ssavar_notifier_function function, void * closure);

void
jive_ssavar_notifier_slot_call(const jive_ssavar_notifier_slot * self, struct jive_ssavar * ssavar);

/* ssavar/input notifiers */

typedef void (*jive_ssavar_input_notifier_function)(void * closure, struct jive_ssavar * ssavar,
	jive::input * input);
typedef struct jive_ssavar_input_notifier jive_ssavar_input_notifier;
typedef struct jive_ssavar_input_notifier_slot jive_ssavar_input_notifier_slot;

struct jive_ssavar_input_notifier_slot {
	struct {
		jive_ssavar_input_notifier * first;
		jive_ssavar_input_notifier * last;
	} notifiers;
	struct jive_context * context;
};

JIVE_EXPORTED_INLINE void
jive_ssavar_input_notifier_slot_init(jive_ssavar_input_notifier_slot * self, jive_context * context)
{
	self->notifiers.first = self->notifiers.last = 0;
	self->context = context;
}

void
jive_ssavar_input_notifier_slot_fini(jive_ssavar_input_notifier_slot * self);

jive_notifier *
jive_ssavar_input_notifier_slot_connect(jive_ssavar_input_notifier_slot * self, jive_ssavar_input_notifier_function function, void * closure);

void
jive_ssavar_input_notifier_slot_call(const jive_ssavar_input_notifier_slot * self,
	struct jive_ssavar * ssavar, jive::input * ssavar_input);

/* ssavar/output notifiers */

typedef void (*jive_ssavar_output_notifier_function)(void * closure, struct jive_ssavar * ssavar,
	jive::output * output);
typedef struct jive_ssavar_output_notifier jive_ssavar_output_notifier;
typedef struct jive_ssavar_output_notifier_slot jive_ssavar_output_notifier_slot;

struct jive_ssavar_output_notifier_slot {
	struct {
		jive_ssavar_output_notifier * first;
		jive_ssavar_output_notifier * last;
	} notifiers;
	struct jive_context * context;
};

JIVE_EXPORTED_INLINE void
jive_ssavar_output_notifier_slot_init(jive_ssavar_output_notifier_slot * self, jive_context * context)
{
	self->notifiers.first = self->notifiers.last = 0;
	self->context = context;
}

void
jive_ssavar_output_notifier_slot_fini(jive_ssavar_output_notifier_slot * self);

jive_notifier *
jive_ssavar_output_notifier_slot_connect(jive_ssavar_output_notifier_slot * self, jive_ssavar_output_notifier_function function, void * closure);

void
jive_ssavar_output_notifier_slot_call(const jive_ssavar_output_notifier_slot * self,
	struct jive_ssavar * ssavar, jive::output * ssavar_output);

/* ssavar divert notifiers */

typedef void (*jive_ssavar_divert_notifier_function)(void * closure, struct jive_ssavar * ssavar,
	jive::output * old_origin, jive::output * new_origin);
typedef struct jive_ssavar_divert_notifier jive_ssavar_divert_notifier;
typedef struct jive_ssavar_divert_notifier_slot jive_ssavar_divert_notifier_slot;

struct jive_ssavar_divert_notifier_slot {
	struct {
		jive_ssavar_divert_notifier * first;
		jive_ssavar_divert_notifier * last;
	} notifiers;
	struct jive_context * context;
};

JIVE_EXPORTED_INLINE void
jive_ssavar_divert_notifier_slot_init(jive_ssavar_divert_notifier_slot * self, jive_context * context)
{
	self->notifiers.first = self->notifiers.last = 0;
	self->context = context;
}

void
jive_ssavar_divert_notifier_slot_fini(jive_ssavar_divert_notifier_slot * self);

jive_notifier *
jive_ssavar_divert_notifier_slot_connect(jive_ssavar_divert_notifier_slot * self, jive_ssavar_divert_notifier_function function, void * closure);

void
jive_ssavar_divert_notifier_slot_call(const jive_ssavar_divert_notifier_slot * self,
	struct jive_ssavar * ssavar, jive::output * old_origin, jive::output * new_origin);

/* ssavar/variable notifiers */

typedef void (*jive_ssavar_variable_notifier_function)(void * closure, struct jive_ssavar * ssavar, struct jive_variable * old_origin, struct jive_variable * new_origin);
typedef struct jive_ssavar_variable_notifier jive_ssavar_variable_notifier;
typedef struct jive_ssavar_variable_notifier_slot jive_ssavar_variable_notifier_slot;

struct jive_ssavar_variable_notifier_slot {
	struct {
		jive_ssavar_variable_notifier * first;
		jive_ssavar_variable_notifier * last;
	} notifiers;
	struct jive_context * context;
};

JIVE_EXPORTED_INLINE void
jive_ssavar_variable_notifier_slot_init(jive_ssavar_variable_notifier_slot * self, jive_context * context)
{
	self->notifiers.first = self->notifiers.last = 0;
	self->context = context;
}

void
jive_ssavar_variable_notifier_slot_fini(jive_ssavar_variable_notifier_slot * self);

jive_notifier *
jive_ssavar_variable_notifier_slot_connect(jive_ssavar_variable_notifier_slot * self, jive_ssavar_variable_notifier_function function, void * closure);

void
jive_ssavar_variable_notifier_slot_call(const jive_ssavar_variable_notifier_slot * self, struct jive_ssavar * ssavar, struct jive_variable * old_origin, struct jive_variable * new_origin);

/* variable/gate notifiers */

typedef void (*jive_variable_gate_notifier_function)(void * closure, struct jive_variable * variable,
	jive::gate * gate);
typedef struct jive_variable_gate_notifier jive_variable_gate_notifier;
typedef struct jive_variable_gate_notifier_slot jive_variable_gate_notifier_slot;

struct jive_variable_gate_notifier_slot {
	struct {
		jive_variable_gate_notifier * first;
		jive_variable_gate_notifier * last;
	} notifiers;
	struct jive_context * context;
};

JIVE_EXPORTED_INLINE void
jive_variable_gate_notifier_slot_init(jive_variable_gate_notifier_slot * self, jive_context * context)
{
	self->notifiers.first = self->notifiers.last = 0;
	self->context = context;
}

void
jive_variable_gate_notifier_slot_fini(jive_variable_gate_notifier_slot * self);

jive_notifier *
jive_variable_gate_notifier_slot_connect(jive_variable_gate_notifier_slot * self, jive_variable_gate_notifier_function function, void * closure);

void
jive_variable_gate_notifier_slot_call(const jive_variable_gate_notifier_slot * self,
	struct jive_variable * variable, jive::gate * variable_gate);

/* variable/resource_class notifiers */

typedef void (*jive_variable_resource_class_notifier_function)(void * closure, struct jive_variable * variable, const struct jive_resource_class * old_rescls, const struct jive_resource_class * new_rescls);
typedef struct jive_variable_resource_class_notifier jive_variable_resource_class_notifier;
typedef struct jive_variable_resource_class_notifier_slot jive_variable_resource_class_notifier_slot;

struct jive_variable_resource_class_notifier_slot {
	struct {
		jive_variable_resource_class_notifier * first;
		jive_variable_resource_class_notifier * last;
	} notifiers;
	struct jive_context * context;
};

JIVE_EXPORTED_INLINE void
jive_variable_resource_class_notifier_slot_init(jive_variable_resource_class_notifier_slot * self, jive_context * context)
{
	self->notifiers.first = self->notifiers.last = 0;
	self->context = context;
}

void
jive_variable_resource_class_notifier_slot_fini(jive_variable_resource_class_notifier_slot * self);

jive_notifier *
jive_variable_resource_class_notifier_slot_connect(jive_variable_resource_class_notifier_slot * self, jive_variable_resource_class_notifier_function function, void * closure);

void
jive_variable_resource_class_notifier_slot_call(const jive_variable_resource_class_notifier_slot * self, struct jive_variable * variable, const struct jive_resource_class * old_rescls, const struct jive_resource_class * new_rescls);

/* variable/resource_name notifiers */

typedef void (*jive_variable_resource_name_notifier_function)(void * closure, struct jive_variable * variable, const struct jive_resource_name * old_rescls, const struct jive_resource_name * new_rescls);
typedef struct jive_variable_resource_name_notifier jive_variable_resource_name_notifier;
typedef struct jive_variable_resource_name_notifier_slot jive_variable_resource_name_notifier_slot;

struct jive_variable_resource_name_notifier_slot {
	struct {
		jive_variable_resource_name_notifier * first;
		jive_variable_resource_name_notifier * last;
	} notifiers;
	struct jive_context * context;
};

JIVE_EXPORTED_INLINE void
jive_variable_resource_name_notifier_slot_init(jive_variable_resource_name_notifier_slot * self, jive_context * context)
{
	self->notifiers.first = self->notifiers.last = 0;
	self->context = context;
}

void
jive_variable_resource_name_notifier_slot_fini(jive_variable_resource_name_notifier_slot * self);

jive_notifier *
jive_variable_resource_name_notifier_slot_connect(jive_variable_resource_name_notifier_slot * self, jive_variable_resource_name_notifier_function function, void * closure);

void
jive_variable_resource_name_notifier_slot_call(const jive_variable_resource_name_notifier_slot * self, struct jive_variable * variable, const struct jive_resource_name * old_rescls, const struct jive_resource_name * new_rescls);

/* region notifiers */

typedef void (*jive_region_notifier_function)(void * closure, struct jive_region * region);
typedef struct jive_region_notifier jive_region_notifier;
typedef struct jive_region_notifier_slot jive_region_notifier_slot;

struct jive_region_notifier_slot {
	struct {
		jive_region_notifier * first;
		jive_region_notifier * last;
	} notifiers;
	struct jive_context * context;
};

JIVE_EXPORTED_INLINE void
jive_region_notifier_slot_init(jive_region_notifier_slot * self, jive_context * context)
{
	self->notifiers.first = self->notifiers.last = 0;
	self->context = context;
}

void
jive_region_notifier_slot_fini(jive_region_notifier_slot * self);

jive_notifier *
jive_region_notifier_slot_connect(jive_region_notifier_slot * self, jive_region_notifier_function function, void * closure);

void
jive_region_notifier_slot_call(const jive_region_notifier_slot * self, struct jive_region * region);

/* region/ssavar notifiers */

typedef void (*jive_region_ssavar_notifier_function)(void * closure, struct jive_region * region, struct jive_ssavar * ssavar);
typedef struct jive_region_ssavar_notifier jive_region_ssavar_notifier;
typedef struct jive_region_ssavar_notifier_slot jive_region_ssavar_notifier_slot;

struct jive_region_ssavar_notifier_slot {
	struct {
		jive_region_ssavar_notifier * first;
		jive_region_ssavar_notifier * last;
	} notifiers;
	struct jive_context * context;
};

JIVE_EXPORTED_INLINE void
jive_region_ssavar_notifier_slot_init(jive_region_ssavar_notifier_slot * self, jive_context * context)
{
	self->notifiers.first = self->notifiers.last = 0;
	self->context = context;
}

void
jive_region_ssavar_notifier_slot_fini(jive_region_ssavar_notifier_slot * self);

jive_notifier *
jive_region_ssavar_notifier_slot_connect(jive_region_ssavar_notifier_slot * self, jive_region_ssavar_notifier_function function, void * closure);

void
jive_region_ssavar_notifier_slot_call(const jive_region_ssavar_notifier_slot * self, struct jive_region * region, struct jive_ssavar * region_ssavar);

/* gate/gate notifiers */

typedef void (*jive_gate_notifier_function)(void * closure, jive::gate * first, jive::gate * second);
typedef struct jive_gate_notifier jive_gate_notifier;
typedef struct jive_gate_notifier_slot jive_gate_notifier_slot;

struct jive_gate_notifier_slot {
	struct {
		jive_gate_notifier * first;
		jive_gate_notifier * last;
	} notifiers;
	struct jive_context * context;
};

JIVE_EXPORTED_INLINE void
jive_gate_notifier_slot_init(jive_gate_notifier_slot * self, jive_context * context)
{
	self->notifiers.first = self->notifiers.last = 0;
	self->context = context;
}

void
jive_gate_notifier_slot_fini(jive_gate_notifier_slot * self);

jive_notifier *
jive_gate_notifier_slot_connect(jive_gate_notifier_slot * self, jive_gate_notifier_function function, void * closure);

void
jive_gate_notifier_slot_call(const jive_gate_notifier_slot * self, jive::gate * first,
	jive::gate * second);

#endif
