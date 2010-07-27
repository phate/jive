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

#endif
