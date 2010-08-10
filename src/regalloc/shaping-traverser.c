#include <jive/regalloc/shaping-traverser.h>
#include <jive/vsdg/traverser-private.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/basetype.h>
#include <jive/util/list.h>

struct jive_shaping_traverser_bucket {
	jive_shaping_traverser * first;
	jive_shaping_traverser * last;
};

struct jive_shaping_region_traverser {
	struct jive_graph * graph;
	struct jive_notifier * node_create, * node_place, * node_destroy, * input_create, * input_change, * input_destroy;
	
	struct jive_traversal_state state_tracker;
	
	/* hash map of regions to set of active nodes */
	struct {
		size_t nitems, nbuckets;
		jive_shaping_traverser_bucket * buckets;
	} regions;
};

struct jive_shaping_traverser {
	jive_traverser base;
	
	jive_region * region;
	struct {
		jive_shaping_traverser * prev;
		jive_shaping_traverser * next;
	} chain;
};

static void
rehash(jive_shaping_region_traverser * self)
{
	size_t new_nbuckets = self->regions.nitems * 2 + 1;
	jive_shaping_traverser_bucket * new_buckets = jive_context_malloc(self->graph->context, new_nbuckets * sizeof(*new_buckets));
	size_t n;
	for(n=0; n<new_nbuckets; n++) new_buckets[n].first = new_buckets[n].last = 0;
	for(n=0; n<self->regions.nbuckets; n++) {
		while(self->regions.buckets[n].first) {
			jive_shaping_traverser * trav = self->regions.buckets[n].first;
			JIVE_LIST_REMOVE(self->regions.buckets[n], trav, chain);
			size_t hash = ((size_t) trav->region) % new_nbuckets;
			JIVE_LIST_PUSH_BACK(new_buckets[hash], trav, chain);
		}
	}
	jive_context_free(self->graph->context, self->regions.buckets);
	self->regions.buckets = new_buckets;
	self->regions.nbuckets = new_nbuckets;
}

static jive_shaping_traverser *
_jive_shaping_region_traverser_get_traverser(jive_shaping_region_traverser * self, jive_region * region)
{
	if (self->regions.nbuckets) {
		size_t hash = ((size_t) region) % self->regions.nbuckets;
		jive_shaping_traverser * match;
		JIVE_LIST_ITERATE(self->regions.buckets[hash], match, chain)
			if (match->region == region) return match;
	}
	
	self->regions.nitems ++;
	if (self->regions.nitems > self->regions.nbuckets) rehash(self);
	
	size_t hash = ((size_t) region) % self->regions.nbuckets;
	jive_shaping_traverser * trav = jive_context_malloc(self->graph->context, sizeof(*trav));
	_jive_traverser_init(&trav->base, self->graph);
	trav->region = region;
	JIVE_LIST_PUSH_BACK(self->regions.buckets[hash], trav, chain);
	
	return trav;
}

static inline void
_jive_shaping_region_traverser_dequeue(jive_shaping_region_traverser * self, jive_node * node)
{
	jive_traversal_nodestate * nodestate = jive_traversal_state_get_nodestate(&self->state_tracker, node);
	if (node->shape_location)
		jive_traversal_state_mark_behind(&self->state_tracker, nodestate);
	else
		jive_traversal_state_mark_ahead(&self->state_tracker, nodestate);
}

static void
_jive_shaping_region_traverser_enqueue(jive_shaping_region_traverser * self, jive_node * node)
{
	jive_traversal_nodestate * nodestate = jive_traversal_state_get_nodestate(&self->state_tracker, node);
	if (jive_traversal_state_is_frontier(&self->state_tracker, nodestate)) return;
	
	jive_shaping_traverser * trav = _jive_shaping_region_traverser_get_traverser(self, node->region);
	jive_traverser_add_frontier(&trav->base, &self->state_tracker, nodestate);
}

static inline void
_jive_shaping_region_traverser_check_next_round(jive_shaping_region_traverser * self, jive_node * node, jive_input * ignore_user, jive_node * ignore_node)
{
	if (node->shape_location) return;
	jive_traversal_nodestate * nodestate = jive_traversal_state_get_nodestate(&self->state_tracker, node);
	if (!jive_traversal_state_is_ahead(&self->state_tracker, nodestate)) return;
	
	size_t n;
	for(n=0; n<node->noutputs; n++) {
		jive_input * user;
		JIVE_LIST_ITERATE(node->outputs[n]->users, user, output_users_list) {
			if ((user == ignore_user) || (node == ignore_node)) continue;
			if (!user->node->shape_location) return;
		}
	}
	
	_jive_shaping_region_traverser_enqueue(self, node);
}

static void
_jive_shaping_region_traverser_node_create(void * closure, jive_node * node)
{
	jive_shaping_region_traverser * self = (jive_shaping_region_traverser *) closure;
	size_t n;
	for(n=0; n<node->ninputs; n++) {
		jive_node * above = node->inputs[n]->origin->node;
		_jive_shaping_region_traverser_dequeue(self, above);
	}
	_jive_shaping_region_traverser_enqueue(self, node);
}

static void
_jive_shaping_region_traverser_node_place(void * closure, jive_node * node)
{
	jive_shaping_region_traverser * self = (jive_shaping_region_traverser *) closure;
	_jive_shaping_region_traverser_dequeue(self, node);
	size_t n;
	for(n=0; n<node->ninputs; n++) {
		jive_node * above = node->inputs[n]->origin->node;
		_jive_shaping_region_traverser_check_next_round(self, above, 0, 0);
	}
}

static void
_jive_shaping_region_traverser_node_destroy(void * closure, jive_node * node)
{
	jive_shaping_region_traverser * self = (jive_shaping_region_traverser *) closure;
	_jive_shaping_region_traverser_dequeue(self, node);
	size_t n;
	for(n=0; n<node->ninputs; n++) {
		jive_input * input = node->inputs[n];
		_jive_shaping_region_traverser_check_next_round(self, input->node, 0, node);
	}
}

static void
_jive_shaping_region_traverser_input_create(void * closure, jive_input * input)
{
	jive_shaping_region_traverser * self = (jive_shaping_region_traverser *) closure;
	if (!input->node->shape_location) _jive_shaping_region_traverser_dequeue(self, input->origin->node);
}

static void
_jive_shaping_region_traverser_input_change(void * closure, jive_input * input, jive_output * old_origin, jive_output * new_origin)
{
	jive_shaping_region_traverser * self = (jive_shaping_region_traverser *) closure;
	if (!input->node->shape_location) {
		_jive_shaping_region_traverser_dequeue(self, new_origin->node);
		_jive_shaping_region_traverser_check_next_round(self, old_origin->node, 0, 0);
	}
}

static void
_jive_shaping_region_traverser_input_destroy(void * closure, jive_input * input)
{
	jive_shaping_region_traverser * self = (jive_shaping_region_traverser *) closure;
	if (!input->node->shape_location)
		_jive_shaping_region_traverser_check_next_round(self, input->origin->node, input, 0);
}

static void
_jive_shaping_region_traverser_init(jive_shaping_region_traverser * self, jive_graph * graph)
{
	jive_traversal_state_init(&self->state_tracker, graph);
	
	self->graph = graph;
	
	self->regions.nitems = self->regions.nbuckets = 0;
	self->regions.buckets = 0;
	
	self->node_create = jive_node_notifier_slot_connect(&graph->on_node_create, _jive_shaping_region_traverser_node_create, self);
	self->node_place = jive_node_notifier_slot_connect(&graph->on_node_place, _jive_shaping_region_traverser_node_place, self);
	self->node_destroy = jive_node_notifier_slot_connect(&graph->on_node_destroy, _jive_shaping_region_traverser_node_destroy, self);
	self->input_create = jive_input_notifier_slot_connect(&graph->on_input_create, _jive_shaping_region_traverser_input_create, self);
	self->input_change = jive_input_change_notifier_slot_connect(&graph->on_input_change, _jive_shaping_region_traverser_input_change, self);
	self->input_destroy = jive_input_notifier_slot_connect(&graph->on_input_destroy, _jive_shaping_region_traverser_input_destroy, self);
	
	jive_node * node;
	JIVE_LIST_ITERATE(graph->bottom, node, graph_bottom_list)
		_jive_shaping_region_traverser_enqueue(self, node);
}

static void
_jive_shaping_region_traverser_fini(jive_shaping_region_traverser * self)
{
	jive_notifier_disconnect(self->node_create);
	jive_notifier_disconnect(self->node_place);
	jive_notifier_disconnect(self->node_destroy);
	jive_notifier_disconnect(self->input_create);
	jive_notifier_disconnect(self->input_change);
	jive_notifier_disconnect(self->input_destroy);
	
	jive_traversal_state_fini(&self->state_tracker);
	
	size_t n;
	for(n=0; n<self->regions.nbuckets; n++) {
		while(self->regions.buckets[n].first) {
			jive_shaping_traverser * trav = self->regions.buckets[n].first;
			JIVE_LIST_REMOVE(self->regions.buckets[n], trav, chain);
			_jive_traverser_fini(&trav->base);
			jive_context_free(self->graph->context, trav);
		}
	}
	
	jive_context_free(self->graph->context, self->regions.buckets);
}

jive_shaping_region_traverser *
jive_shaping_region_traverser_create(struct jive_graph * graph)
{
	jive_shaping_region_traverser * self = jive_context_malloc(graph->context, sizeof(*self));
	_jive_shaping_region_traverser_init(self, graph);
	
	return self;
}

void
jive_shaping_region_traverser_destroy(jive_shaping_region_traverser * self)
{
	_jive_shaping_region_traverser_fini(self);
	jive_context_free(self->graph->context, self);
}

struct jive_traverser *
jive_shaping_region_traverser_enter_region(jive_shaping_region_traverser * self, struct jive_region * region)
{
	jive_shaping_traverser * trav = _jive_shaping_region_traverser_get_traverser(self, region);
	return &trav->base;
}
