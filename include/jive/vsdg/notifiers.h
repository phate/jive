#ifndef JIVE_VSDG_NOTIFIERS_H
#define JIVE_VSDG_NOTIFIERS_H

#include <jive/context.h>

struct jive_node;
struct jive_input;
struct jive_output;

typedef struct jive_notifier jive_notifier;
typedef struct jive_notifier_class jive_notifier_class;

struct jive_notifier_class {
	void (*disconnect)(jive_notifier * self);
};

struct jive_notifier {
	const jive_notifier_class * class_;
	struct jive_context * context;
};

static inline void
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

static inline void
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

/* input notifiers */

typedef void (*jive_input_notifier_function)(void * closure, struct jive_input * input);
typedef struct jive_input_notifier jive_input_notifier;
typedef struct jive_input_notifier_slot jive_input_notifier_slot;

struct jive_input_notifier_slot {
	struct {
		jive_input_notifier * first;
		jive_input_notifier * last;
	} notifiers;
	struct jive_context * context;
};

static inline void
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
jive_input_notifier_slot_call(const jive_input_notifier_slot * self, struct jive_input * input);

/* input_change notifiers */

typedef void (*jive_input_change_notifier_function)(void * closure, struct jive_input * input, struct jive_output * old_origin, struct jive_output * new_origin);
typedef struct jive_input_change_notifier jive_input_change_notifier;
typedef struct jive_input_change_notifier_slot jive_input_change_notifier_slot;

struct jive_input_change_notifier_slot {
	struct {
		jive_input_change_notifier * first;
		jive_input_change_notifier * last;
	} notifiers;
	struct jive_context * context;
};

static inline void
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
jive_input_change_notifier_slot_call(const jive_input_change_notifier_slot * self, struct jive_input * input, struct jive_output * old_origin, struct jive_output * new_origin);

/* output notifiers */

typedef void (*jive_output_notifier_function)(void * closure, struct jive_output * output);
typedef struct jive_output_notifier jive_output_notifier;
typedef struct jive_output_notifier_slot jive_output_notifier_slot;

struct jive_output_notifier_slot {
	struct {
		jive_output_notifier * first;
		jive_output_notifier * last;
	} notifiers;
	struct jive_context * context;
};

static inline void
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
jive_output_notifier_slot_call(const jive_output_notifier_slot * self, struct jive_output * output);

#endif
