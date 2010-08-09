#include <jive/vsdg/traversal-state-private.h>

void
_jive_traversal_state_init_slow(jive_traversal_state * self, jive_graph * graph)
{
	size_t n = graph->ntraverser_slots;
	self->index = n;
	
	graph->traverser_slots = jive_context_realloc(graph->context,
		graph->traverser_slots, sizeof(graph->traverser_slots[0]) * (n+1));
	graph->ntraverser_slots = n + 1;
	
	graph->traverser_slots[n].cookie = 0;
	self->cookie = graph->traverser_slots[n].cookie + 1;
	graph->traverser_slots[n].traverser = (void *) self; /* FIXME: data type */
}

jive_traversal_nodestate *
jive_traversal_state_alloc_nodestate(const jive_traversal_state * self, jive_node * node)
{
	jive_traversal_nodestate * nodestate = jive_context_malloc(node->graph->context, sizeof(*nodestate));
	if (unlikely(self->index >= node->ntraverser_slots)) {
		node->traverser_slots = jive_context_realloc(node->graph->context, node->traverser_slots,
			sizeof(nodestate) * (self->index + 1));
		size_t n;
		for(n = node->ntraverser_slots; n<self->index; n++)
			node->traverser_slots[n] = 0;
		node->ntraverser_slots = self->index + 1;
	}
	
	nodestate->node = node;
	nodestate->cookie = self->cookie - 1;
	nodestate->traverser = 0;
	
	node->traverser_slots[self->index] = nodestate;
	return nodestate;
}
