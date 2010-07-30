#include <jive/vsdg/notifiers.h>
#include <jive/util/list.h>

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


