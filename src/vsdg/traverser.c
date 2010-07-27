#include <jive/vsdg/traverser.h>
#include <jive/vsdg/traverser-private.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/types.h>

#include <jive/debug-private.h>
#include <jive/util/list.h>
#include <string.h>

static const jive_traverser_class JIVE_TRAVERSER = {
	.parent = 0,
	.fini = _jive_traverser_fini,
	.next = 0
};

void
jive_traverser_destroy(jive_traverser * self)
{
	self->class_->fini(self);
	jive_context_free(self->graph->context, self);
}

void
_jive_traverser_fini(jive_traverser * self)
{
	self->graph->traverser_slots[self->index].cookie = self->cookie + 1;
	/* TODO: reset cookies iff == 0 ? or don't use this slot? */
	DEBUG_ASSERT(self->graph->traverser_slots[self->index].cookie);
	self->graph->traverser_slots[self->index].traverser = 0;
	
	if (self->node_create) jive_notifier_disconnect(self->node_create);
	if (self->node_destroy) jive_notifier_disconnect(self->node_destroy);
	if (self->input_change) jive_notifier_disconnect(self->input_change);
}

void
_jive_traverser_init(jive_traverser * self, jive_graph * graph)
{
	self->graph = graph;
	self->next_nodes.first = self->next_nodes.last = 0;
	self->visited_nodes.first = self->visited_nodes.last = 0;
	
	self->node_create = 0;
	self->node_destroy = 0;
	self->input_change = 0;
	
	size_t n;
	for(n = 0; n < graph->ntraverser_slots; n++) {
		if (!graph->traverser_slots[n].traverser) {
			self->index = n;
			self->cookie = graph->traverser_slots[n].cookie + 1;
			graph->traverser_slots[n].traverser = self;
			return;
		}
	}
	
	n = graph->ntraverser_slots;
	
	graph->traverser_slots = jive_context_realloc(graph->context,
		graph->traverser_slots, sizeof(graph->traverser_slots[0]) * (n+1));
	graph->ntraverser_slots = n + 1;
	
	graph->traverser_slots[n].cookie = 0;
	self->cookie = graph->traverser_slots[n].cookie + 1;
	graph->traverser_slots[n].traverser = self;
}

jive_traverser_nodestate *
jive_traverser_alloc_nodestate(const jive_traverser * self, jive_node * node)
{
	jive_traverser_nodestate * nodestate = jive_context_malloc(node->graph->context, sizeof(*nodestate));
	if (unlikely(self->index >= node->ntraverser_slots)) {
		node->traverser_slots = jive_context_realloc(node->graph->context, node->traverser_slots,
			sizeof(nodestate) * (self->index + 1));
		memset(node->traverser_slots + node->ntraverser_slots, 0,
			(self->index - node->ntraverser_slots) * sizeof(nodestate));
		node->ntraverser_slots = self->index + 1;
	}
	
	nodestate->node = node;
	nodestate->cookie = self->cookie - 1;
	
	node->traverser_slots[self->index] = nodestate;
	return nodestate;
}

bool
jive_traverser_node_is_unvisited(const jive_traverser * self, jive_node * node)
{
	return jive_traverser_get_nodestate(self, node)->cookie < self->cookie;
}

bool
jive_traverser_node_is_candidate(const jive_traverser * self, jive_node * node)
{
	return jive_traverser_get_nodestate(self, node)->cookie == self->cookie;
}

bool
jive_traverser_node_is_visited(const jive_traverser * self, jive_node * node)
{
	return jive_traverser_get_nodestate(self, node)->cookie > self->cookie;
}

static inline void
jive_traverser_nodestate_unlink(jive_traverser * self, jive_traverser_nodestate * nodestate)
{
	if (nodestate->cookie == self->cookie) {
		JIVE_LIST_REMOVE(self->next_nodes, nodestate, traverser_node_list);
	} else if (nodestate->cookie > self->cookie) {
		JIVE_LIST_REMOVE(self->visited_nodes, nodestate, traverser_node_list);
	}
}

static inline void
jive_traverser_mark_node_unvisited(jive_traverser * self, jive_node * node)
{
	jive_traverser_nodestate * nodestate = jive_traverser_get_nodestate(self, node);
	jive_traverser_nodestate_unlink(self, nodestate);
	nodestate->cookie = self->cookie - 1;
}

static inline void
jive_traverser_mark_node_candidate(jive_traverser * self, jive_node * node)
{
	jive_traverser_nodestate * nodestate = jive_traverser_get_nodestate(self, node);
	jive_traverser_nodestate_unlink(self, nodestate);
	nodestate->cookie = self->cookie;
	JIVE_LIST_PUSHBACK(self->next_nodes, nodestate, traverser_node_list);
}

static inline void
jive_traverser_mark_node_visited(jive_traverser * self, jive_node * node)
{
	jive_traverser_nodestate * nodestate = jive_traverser_get_nodestate(self, node);
	jive_traverser_nodestate_unlink(self, nodestate);
	nodestate->cookie = self->cookie + 1;
	JIVE_LIST_PUSHBACK(self->visited_nodes, nodestate, traverser_node_list);
}

/* topdown traversal */

static inline bool
predecessors_visited(jive_traverser * self, jive_node * node)
{
	size_t n;
	for(n=0; n<node->ninputs; n++)
		if (!jive_traverser_node_is_visited(self, node->inputs[n]->origin->node)) return false;
	return true;
}

static inline void
_jive_topdown_traverser_check_node(jive_traverser * self, jive_node * node)
{
	if (!jive_traverser_node_is_unvisited(self, node)) return;
	if (predecessors_visited(self, node))
		jive_traverser_mark_node_candidate(self, node);
}

static void
_jive_topdown_traverser_node_create(void * closure, jive_node * node)
{
	jive_traverser * self = closure;
	if (predecessors_visited(self, node))
		jive_traverser_mark_node_visited(self, node);
}

static void
_jive_topdown_traverser_node_destroy(void * closure, jive_node * node)
{
	jive_traverser * self = closure;
	jive_traverser_mark_node_unvisited(self, node);
}

static inline void
_jive_topdown_traverser_init(jive_traverser * self, jive_graph * graph)
{
	_jive_traverser_init(self, graph);
	
	jive_node * node;
	JIVE_LIST_ITERATE(graph->top, node, graph_top_list)
		jive_traverser_mark_node_candidate(self, node);
	
	self->node_create = jive_node_notifier_slot_connect(&graph->on_node_create, _jive_topdown_traverser_node_create, self);
	self->node_destroy = jive_node_notifier_slot_connect(&graph->on_node_destroy, _jive_topdown_traverser_node_destroy, self);
}

static jive_node *
_jive_topdown_traverser_next(jive_traverser * self)
{
	if (!self->next_nodes.first) return 0;
	jive_node * node = self->next_nodes.first->node;
	jive_traverser_mark_node_visited(self, node);
	
	size_t n;
	for(n=0; n<node->noutputs; n++) {
		jive_input * user;
		JIVE_LIST_ITERATE(node->outputs[n]->users, user, output_users_list)
			_jive_topdown_traverser_check_node(self, user->node);
	}
	
	return node;
}

static const jive_traverser_class JIVE_TOPDOWN_TRAVERSER = {
	.parent = &JIVE_TRAVERSER,
	.fini = _jive_traverser_fini,
	.next = _jive_topdown_traverser_next
};

jive_traverser *
jive_topdown_traverser_create(jive_graph * graph)
{
	jive_traverser * traverser = jive_context_malloc(graph->context, sizeof(*traverser));
	traverser->class_ = &JIVE_TOPDOWN_TRAVERSER;
	_jive_topdown_traverser_init(traverser, graph);
	return traverser;
}

static inline void
_jive_bottomup_traverser_init(jive_traverser * self, jive_graph * graph)
{
	_jive_traverser_init(self, graph);
	
	jive_node * node;
	JIVE_LIST_ITERATE(graph->bottom, node, graph_bottom_list)
		jive_traverser_mark_node_candidate(self, node);
}

static inline void
_jive_bottomup_traverser_check_node(jive_traverser * self, jive_node * node)
{
	if (!jive_traverser_node_is_unvisited(self, node)) return;
	size_t n;
	for(n=0; n<node->noutputs; n++) {
		jive_input * user;
		JIVE_LIST_ITERATE(node->outputs[n]->users, user, output_users_list) {
			if (jive_traverser_node_is_unvisited(self, user->node)) return;
		}
	}
	jive_traverser_mark_node_candidate(self, node);
}

static jive_node *
_jive_bottomup_traverser_next(jive_traverser * self)
{
	if (!self->next_nodes.first) return 0;
	jive_node * node = self->next_nodes.first->node;
	jive_traverser_mark_node_visited(self, node);
	
	size_t n;
	for(n=0; n<node->ninputs; n++) {
		_jive_bottomup_traverser_check_node(self, node->inputs[n]->origin->node);
	}
	
	return node;
}

static const jive_traverser_class JIVE_BOTTOMUP_TRAVERSER = {
	.parent = &JIVE_TRAVERSER,
	.fini = _jive_traverser_fini,
	.next = _jive_bottomup_traverser_next
};

jive_traverser *
jive_bottomup_traverser_create(jive_graph * graph)
{
	jive_traverser * traverser = jive_context_malloc(graph->context, sizeof(*traverser));
	traverser->class_ = &JIVE_BOTTOMUP_TRAVERSER;
	_jive_bottomup_traverser_init(traverser, graph);
	return traverser;
}
