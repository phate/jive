#include <jive/vsdg/notifiers.h>
#include <jive/util/list.h>

/* node notifiers */

struct jive_node_notifier {
	jive_notifier base;
	jive_node_notifier_function function;
	void * closure;
	jive_node_notifier_slot * slot;
	jive_context * context;
	struct {
		jive_node_notifier * prev;
		jive_node_notifier * next;
	} notifier_list;
};

static inline void
jive_node_notifier_unlink(jive_node_notifier * self)
{
	if (!self->slot) return;
	
	JIVE_LIST_REMOVE(self->slot->notifiers, self, notifier_list);
	self->slot = 0;
}

static void
_jive_node_notifier_disconnect(jive_notifier * _self)
{
	jive_node_notifier * self = (jive_node_notifier *) _self;
	jive_node_notifier_unlink(self);
	jive_context_free(self->context, self);
}

static const jive_notifier_class JIVE_NODE_NOTIFIER = {
	.disconnect = _jive_node_notifier_disconnect
};

void
jive_node_notifier_slot_fini(jive_node_notifier_slot * self)
{
	while(self->notifiers.first) jive_node_notifier_unlink(self->notifiers.first);
}

jive_notifier *
jive_node_notifier_slot_connect(jive_node_notifier_slot * self, jive_node_notifier_function function, void * closure)
{
	jive_node_notifier * notifier = jive_context_malloc(self->context, sizeof(*notifier));
	notifier->base.class_ = &JIVE_NODE_NOTIFIER;
	notifier->slot = self;
	notifier->context = self->context;
	notifier->function = function;
	notifier->closure = closure;
	JIVE_LIST_PUSH_BACK(self->notifiers, notifier, notifier_list);
	
	return &notifier->base;
}

void
jive_node_notifier_slot_call(const jive_node_notifier_slot * self, struct jive_node * node)
{
	jive_node_notifier * notifier;
	JIVE_LIST_ITERATE(self->notifiers, notifier, notifier_list)
		notifier->function(notifier->closure, node);
}

/* input notifiers */

struct jive_input_notifier {
	jive_notifier base;
	jive_input_notifier_function function;
	void * closure;
	jive_input_notifier_slot * slot;
	jive_context * context;
	struct {
		jive_input_notifier * prev;
		jive_input_notifier * next;
	} notifier_list;
};

static inline void
jive_input_notifier_unlink(jive_input_notifier * self)
{
	if (!self->slot) return;
	
	JIVE_LIST_REMOVE(self->slot->notifiers, self, notifier_list);
	self->slot = 0;
}

static void
_jive_input_notifier_disconnect(jive_notifier * _self)
{
	jive_input_notifier * self = (jive_input_notifier *) _self;
	jive_input_notifier_unlink(self);
	jive_context_free(self->context, self);
}

static const jive_notifier_class JIVE_INPUT_NOTIFIER = {
	.disconnect = _jive_input_notifier_disconnect
};

void
jive_input_notifier_slot_fini(jive_input_notifier_slot * self)
{
	while(self->notifiers.first) jive_input_notifier_unlink(self->notifiers.first);
}

jive_notifier *
jive_input_notifier_slot_connect(jive_input_notifier_slot * self, jive_input_notifier_function function, void * closure)
{
	jive_input_notifier * notifier = jive_context_malloc(self->context, sizeof(*notifier));
	notifier->base.class_ = &JIVE_INPUT_NOTIFIER;
	notifier->slot = self;
	notifier->context = self->context;
	notifier->function = function;
	notifier->closure = closure;
	JIVE_LIST_PUSH_BACK(self->notifiers, notifier, notifier_list);
	
	return &notifier->base;
}

void
jive_input_notifier_slot_call(const jive_input_notifier_slot * self, struct jive_input * input)
{
	jive_input_notifier * notifier;
	JIVE_LIST_ITERATE(self->notifiers, notifier, notifier_list)
		notifier->function(notifier->closure, input);
}

/* input_change notifiers */

struct jive_input_change_notifier {
	jive_notifier base;
	jive_input_change_notifier_function function;
	void * closure;
	jive_input_change_notifier_slot * slot;
	jive_context * context;
	struct {
		jive_input_change_notifier * prev;
		jive_input_change_notifier * next;
	} notifier_list;
};

static inline void
jive_input_change_notifier_unlink(jive_input_change_notifier * self)
{
	if (!self->slot) return;
	
	JIVE_LIST_REMOVE(self->slot->notifiers, self, notifier_list);
	self->slot = 0;
}

static void
_jive_input_change_notifier_disconnect(jive_notifier * _self)
{
	jive_input_change_notifier * self = (jive_input_change_notifier *) _self;
	jive_input_change_notifier_unlink(self);
	jive_context_free(self->context, self);
}

static const jive_notifier_class JIVE_INPUT_CHANGE_NOTIFIER = {
	.disconnect = _jive_input_change_notifier_disconnect
};

void
jive_input_change_notifier_slot_fini(jive_input_change_notifier_slot * self)
{
	while(self->notifiers.first) jive_input_change_notifier_unlink(self->notifiers.first);
}

jive_notifier *
jive_input_change_notifier_slot_connect(jive_input_change_notifier_slot * self, jive_input_change_notifier_function function, void * closure)
{
	jive_input_change_notifier * notifier = jive_context_malloc(self->context, sizeof(*notifier));
	notifier->base.class_ = &JIVE_INPUT_CHANGE_NOTIFIER;
	notifier->slot = self;
	notifier->context = self->context;
	notifier->function = function;
	notifier->closure = closure;
	JIVE_LIST_PUSH_BACK(self->notifiers, notifier, notifier_list);
	
	return &notifier->base;
}

void
jive_input_change_notifier_slot_call(const jive_input_change_notifier_slot * self, struct jive_input * input, struct jive_output * old_origin, struct jive_output * new_origin)
{
	jive_input_change_notifier * notifier;
	JIVE_LIST_ITERATE(self->notifiers, notifier, notifier_list)
		notifier->function(notifier->closure, input, old_origin, new_origin);
}

/* output notifiers */

struct jive_output_notifier {
	jive_notifier base;
	jive_output_notifier_function function;
	void * closure;
	jive_output_notifier_slot * slot;
	jive_context * context;
	struct {
		jive_output_notifier * prev;
		jive_output_notifier * next;
	} notifier_list;
};

static inline void
jive_output_notifier_unlink(jive_output_notifier * self)
{
	if (!self->slot) return;
	
	JIVE_LIST_REMOVE(self->slot->notifiers, self, notifier_list);
	self->slot = 0;
}

static void
_jive_output_notifier_disconnect(jive_notifier * _self)
{
	jive_output_notifier * self = (jive_output_notifier *) _self;
	jive_output_notifier_unlink(self);
	jive_context_free(self->context, self);
}

static const jive_notifier_class JIVE_OUTPUT_NOTIFIER = {
	.disconnect = _jive_output_notifier_disconnect
};

void
jive_output_notifier_slot_fini(jive_output_notifier_slot * self)
{
	while(self->notifiers.first) jive_output_notifier_unlink(self->notifiers.first);
}

jive_notifier *
jive_output_notifier_slot_connect(jive_output_notifier_slot * self, jive_output_notifier_function function, void * closure)
{
	jive_output_notifier * notifier = jive_context_malloc(self->context, sizeof(*notifier));
	notifier->base.class_ = &JIVE_OUTPUT_NOTIFIER;
	notifier->slot = self;
	notifier->context = self->context;
	notifier->function = function;
	notifier->closure = closure;
	JIVE_LIST_PUSH_BACK(self->notifiers, notifier, notifier_list);
	
	return &notifier->base;
}

void
jive_output_notifier_slot_call(const jive_output_notifier_slot * self, struct jive_output * output)
{
	jive_output_notifier * notifier;
	JIVE_LIST_ITERATE(self->notifiers, notifier, notifier_list)
		notifier->function(notifier->closure, output);
}


