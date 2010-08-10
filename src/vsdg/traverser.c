#include <jive/vsdg/traverser.h>
#include <jive/vsdg/traverser-private.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/basetype.h>

#include <jive/debug-private.h>
#include <jive/util/list.h>
#include <string.h>

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

const jive_traverser_class JIVE_TRAVERSER = {
	.parent = 0,
	.fini = _jive_traverser_fini,
	.next = 0
};

void
jive_traverser_destroy(jive_traverser * self)
{
	jive_context * context = self->graph->context;
	self->class_->fini(self);
	jive_context_free(context, self);
}

void
_jive_traverser_fini(jive_traverser * self)
{
}

void
_jive_traverser_init(jive_traverser * self, jive_graph * graph)
{
	self->graph = graph;
	self->frontier.first = self->frontier.last = 0;
}

void
jive_traverser_add_frontier(jive_traverser * self, jive_traversal_state * state_tracker, jive_traversal_nodestate * nodestate)
{
	jive_traversal_state_mark_frontier(state_tracker, nodestate);
	JIVE_LIST_PUSH_BACK(self->frontier, nodestate, traverser_node_list);
	nodestate->traverser = self;
}

/* full graph traversal */

const jive_traverser_class JIVE_FULL_TRAVERSER = {
	.parent = &JIVE_TRAVERSER,
	.fini = _jive_full_traverser_fini,
	.next = 0
};

void
_jive_full_traverser_fini(jive_traverser * self_)
{
	jive_full_traverser * self = (jive_full_traverser *) self_;
	jive_traversal_state_fini(&self->state_tracker);
	
	if (self->node_create) jive_notifier_disconnect(self->node_create);
	if (self->node_destroy) jive_notifier_disconnect(self->node_destroy);
	if (self->input_change) jive_notifier_disconnect(self->input_change);
	
	_jive_traverser_fini(&self->base);
}

void
_jive_full_traverser_init(jive_full_traverser * self, jive_graph * graph)
{
	_jive_traverser_init(&self->base, graph);
	
	self->node_create = 0;
	self->node_destroy = 0;
	self->input_change = 0;
	
	jive_traversal_state_init(&self->state_tracker, graph);
}

void
jive_full_traverser_add_frontier(jive_full_traverser * self, jive_traversal_nodestate * nodestate)
{
	jive_traverser_add_frontier(&self->base, &self->state_tracker, nodestate);
}

/* topdown traversal */

static inline bool
predecessors_visited(const jive_full_traverser * self, jive_node * node)
{
	size_t n;
	for(n=0; n<node->ninputs; n++) {
		jive_traversal_nodestate * nodestate = jive_traversal_state_get_nodestate(&self->state_tracker, node->inputs[n]->origin->node);
		if (!jive_traversal_state_is_behind(&self->state_tracker, nodestate)) return false;
	}
	return true;
}

static inline void
_jive_topdown_traverser_check_node(jive_full_traverser * self, jive_node * node, jive_traversal_nodestate * nodestate)
{
	if (!jive_traversal_state_is_ahead(&self->state_tracker, nodestate)) return;
	if (predecessors_visited(self, node))
		jive_full_traverser_add_frontier(self, nodestate);
}

static void
_jive_topdown_traverser_node_create(void * closure, jive_node * node)
{
	jive_full_traverser * self = closure;
	if (predecessors_visited(self, node)) {
		jive_traversal_nodestate * nodestate = jive_traversal_state_get_nodestate(&self->state_tracker, node);
		jive_traversal_state_mark_behind(&self->state_tracker, nodestate);
	}
}

static void
_jive_topdown_traverser_node_destroy(void * closure, jive_node * node)
{
	/* Only "bottom" nodes can be destroyed, therefore it is
	sufficient to simply remove this node from this traversers 
	records since it cannot influence the state of any other node. */
	jive_full_traverser * self = closure;
	jive_traversal_nodestate * nodestate = jive_traversal_state_get_nodestate(&self->state_tracker, node);
	jive_traversal_state_forget(&self->state_tracker, nodestate);
}

static void
_jive_topdown_traverser_input_change(void * closure, jive_input * input, jive_output * old_origin, jive_output * new_origin)
{
	jive_full_traverser * self = closure;
	jive_traversal_nodestate * nodestate = jive_traversal_state_get_nodestate(&self->state_tracker, input->node);
	if (jive_traversal_state_is_frontier(&self->state_tracker, nodestate)) {
		jive_traversal_state_mark_ahead(&self->state_tracker, nodestate);
		_jive_topdown_traverser_check_node(self, input->node, nodestate);
	} else if (jive_traversal_state_is_ahead(&self->state_tracker, nodestate)) {
		_jive_topdown_traverser_check_node(self, input->node, nodestate);
	}
}

static inline void
_jive_topdown_traverser_init(jive_full_traverser * self, jive_graph * graph)
{
	_jive_full_traverser_init(self, graph);
	
	jive_node * node;
	JIVE_LIST_ITERATE(graph->top, node, graph_top_list) {
		jive_traversal_nodestate * nodestate = jive_traversal_state_get_nodestate(&self->state_tracker, node);
		jive_full_traverser_add_frontier(self, nodestate);
	}
	
	self->node_create = jive_node_notifier_slot_connect(&graph->on_node_create, _jive_topdown_traverser_node_create, self);
	self->node_destroy = jive_node_notifier_slot_connect(&graph->on_node_destroy, _jive_topdown_traverser_node_destroy, self);
	self->input_change = jive_input_change_notifier_slot_connect(&graph->on_input_change, _jive_topdown_traverser_input_change, self);
}

static jive_node *
_jive_topdown_traverser_next(jive_traverser * self_)
{
	jive_full_traverser * self = (jive_full_traverser *) self_;
	if (!self->base.frontier.first) return 0;
	jive_node * node = self->base.frontier.first->node;
	jive_traversal_nodestate * nodestate = jive_traversal_state_get_nodestate(&self->state_tracker, node);
	jive_traversal_state_mark_behind(&self->state_tracker, nodestate);
	
	size_t n;
	for(n=0; n<node->noutputs; n++) {
		jive_input * user;
		JIVE_LIST_ITERATE(node->outputs[n]->users, user, output_users_list) {
			jive_traversal_nodestate * nodestate = jive_traversal_state_get_nodestate(&self->state_tracker, user->node);
			_jive_topdown_traverser_check_node(self, user->node, nodestate);
		}
	}
	
	return node;
}

static const jive_traverser_class JIVE_TOPDOWN_TRAVERSER = {
	.parent = &JIVE_FULL_TRAVERSER,
	.fini = _jive_full_traverser_fini,
	.next = _jive_topdown_traverser_next
};

jive_traverser *
jive_topdown_traverser_create(jive_graph * graph)
{
	jive_full_traverser * traverser = jive_context_malloc(graph->context, sizeof(*traverser));
	traverser->base.class_ = &JIVE_TOPDOWN_TRAVERSER;
	_jive_topdown_traverser_init(traverser, graph);
	return &traverser->base;
}

/* bottom-up traversal */

static void
_jive_bottomup_traverser_node_create(void * closure, jive_node * node)
{
	/* all newly created nodes are at the bottom of the graph,
	therefore we can simply treat them as "visited" */
	jive_full_traverser * self = closure;
	jive_traversal_nodestate * nodestate = jive_traversal_state_get_nodestate(&self->state_tracker, node);
	jive_traversal_state_mark_behind(&self->state_tracker, nodestate);
}

static inline void
_jive_bottomup_traverser_check_node(jive_full_traverser * self, jive_node * node, jive_traversal_nodestate * nodestate)
{
	if (!jive_traversal_state_is_ahead(&self->state_tracker, nodestate)) return;
	size_t n;
	for(n=0; n<node->noutputs; n++) {
		jive_input * user;
		JIVE_LIST_ITERATE(node->outputs[n]->users, user, output_users_list) {
			jive_traversal_nodestate * tmp = jive_traversal_state_get_nodestate(&self->state_tracker, user->node);
			if (!jive_traversal_state_is_behind(&self->state_tracker, tmp)) return;
		}
	}
	jive_full_traverser_add_frontier(self, nodestate);
}

static void
_jive_bottomup_traverser_node_destroy(void * closure, jive_node * node)
{
	jive_full_traverser * self = closure;
	/* This node is about to be removed; if it was visited already,
	there is nothing to do, all predecessors are already being tracked.
	If we are however removing a node that has not been visited yet,
	the predecessors may become traversable now. */
	jive_traversal_nodestate * nodestate = jive_traversal_state_get_nodestate(&self->state_tracker, node);
	if (!jive_traversal_state_is_behind(&self->state_tracker, nodestate)) {
		jive_traversal_state_mark_behind(&self->state_tracker, nodestate);
		size_t n;
		for(n=0; n<node->ninputs; n++) {
			jive_traversal_nodestate * tmp = jive_traversal_state_get_nodestate(&self->state_tracker, node->inputs[n]->origin->node);
			_jive_bottomup_traverser_check_node(self, node->inputs[n]->origin->node, tmp);
		}
	}
	
	/* Now purge this node from our records. */
	jive_traversal_state_forget(&self->state_tracker, nodestate);
}

static void
_jive_bottomup_traverser_input_change(void * closure, jive_input * input, jive_output * old_origin, jive_output * new_origin)
{
	jive_full_traverser * self = closure;
	
	jive_traversal_nodestate * nodestate = jive_traversal_state_get_nodestate(&self->state_tracker, old_origin->node);
	_jive_bottomup_traverser_check_node(self, old_origin->node, nodestate);
	
	nodestate = jive_traversal_state_get_nodestate(&self->state_tracker, new_origin->node);
	if (jive_traversal_state_is_frontier(&self->state_tracker, nodestate)) {
		jive_traversal_state_mark_ahead(&self->state_tracker, nodestate);
		_jive_bottomup_traverser_check_node(self, new_origin->node, nodestate);
	}
}

static inline void
_jive_bottomup_traverser_init(jive_full_traverser * self, jive_graph * graph)
{
	_jive_full_traverser_init(self, graph);
	
	jive_node * node;
	JIVE_LIST_ITERATE(graph->bottom, node, graph_bottom_list) {
		jive_traversal_nodestate * nodestate = jive_traversal_state_get_nodestate(&self->state_tracker, node);
		jive_full_traverser_add_frontier(self, nodestate);
	}
	
	self->node_create = jive_node_notifier_slot_connect(&graph->on_node_create, _jive_bottomup_traverser_node_create, self);
	self->node_destroy = jive_node_notifier_slot_connect(&graph->on_node_destroy, _jive_bottomup_traverser_node_destroy, self);
	self->input_change = jive_input_change_notifier_slot_connect(&graph->on_input_change, _jive_bottomup_traverser_input_change, self);
}

static jive_node *
_jive_bottomup_traverser_next(jive_traverser * self_)
{
	jive_full_traverser * self = (jive_full_traverser *) self_;
	
	if (!self->base.frontier.first) return 0;
	jive_node * node = self->base.frontier.first->node;
	jive_traversal_nodestate * nodestate = jive_traversal_state_get_nodestate(&self->state_tracker, node);
	jive_traversal_state_mark_behind(&self->state_tracker, nodestate);
	
	size_t n;
	for(n=0; n<node->ninputs; n++) {
		jive_traversal_nodestate * nodestate = jive_traversal_state_get_nodestate(&self->state_tracker, node->inputs[n]->origin->node);
		_jive_bottomup_traverser_check_node(self, node->inputs[n]->origin->node, nodestate);
	}
	
	return node;
}

static const jive_traverser_class JIVE_BOTTOMUP_TRAVERSER = {
	.parent = &JIVE_FULL_TRAVERSER,
	.fini = _jive_full_traverser_fini,
	.next = _jive_bottomup_traverser_next
};

jive_traverser *
jive_bottomup_traverser_create(jive_graph * graph)
{
	jive_full_traverser * traverser = jive_context_malloc(graph->context, sizeof(*traverser));
	traverser->base.class_ = &JIVE_BOTTOMUP_TRAVERSER;
	_jive_bottomup_traverser_init(traverser, graph);
	return &traverser->base;
}
