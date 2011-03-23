#include <jive/regalloc/shaped-graph.h>

#include <jive/vsdg/graph.h>

jive_shaped_graph *
jive_shaped_graph_create(jive_graph * graph)
{
	jive_context * context = graph->context;
	jive_shaped_graph * self = jive_context_malloc(context, sizeof(*self));
	
	self->graph = graph;
	self->context = context;
	
	size_t n;
	for(n = 0; n < sizeof(self->callbacks)/sizeof(self->callbacks[0]); n++)
		self->callbacks[n] = NULL;
	
	return self;
}

void
jive_shaped_graph_destroy(jive_shaped_graph * self)
{
	jive_context * context = self->context;
	size_t n;
	for(n = 0; n < sizeof(self->callbacks)/sizeof(self->callbacks[0]); n++) {
		if (self->callbacks[n])
			jive_notifier_disconnect(self->callbacks[n]);
	}
	
	jive_context_free(context, self);
}

