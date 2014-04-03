/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/traverser.h>
#include <jive/vsdg/traverser-private.h>

#include <string.h>

#include <jive/common.h>
#include <jive/util/list.h>
#include <jive/vsdg/basetype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/tracker.h>
#include <jive/vsdg/tracker-private.h>

struct jive_traverser {
	const jive_traverser_class * class_;
	jive_graph * graph;
};

struct jive_traverser_class {
	void (*fini)(jive_traverser * self);
	jive_node * (*next)(jive_traverser * self);
};

void
jive_traverser_destroy(jive_traverser * self)
{
	self->class_->fini(self);
	jive_context_free(self->graph->context, self);
}

jive_node *
jive_traverser_next(jive_traverser * self)
{
	return self->class_->next(self);
}

typedef struct jive_full_traverser jive_full_traverser;
struct jive_full_traverser {
	jive_traverser base;
	jive_traversal_tracker tracker;
	
	jive_notifier * callbacks[3];
	size_t ncallbacks;
};

static void
jive_full_traverser_init(jive_full_traverser * self, jive_graph * graph)
{
	self->base.graph = graph;
	jive_traversal_tracker_init(&self->tracker, graph);
	self->ncallbacks = 0;
}

static void
jive_full_traverser_fini(jive_traverser * self_)
{
	jive_full_traverser * self = (jive_full_traverser *) self_;
	jive_traversal_tracker_fini(&self->tracker);
	size_t n;
	for (n = 0; n < self->ncallbacks; n++)
		jive_notifier_disconnect(self->callbacks[n]);
}

static bool
jive_topdown_traverser_predecessors_visited(jive_full_traverser * self, jive_node * node)
{
	size_t n;
	for (n = 0; n < node->ninputs; n++) {
		jive_input * input = node->inputs[n];
		if (jive_traversal_tracker_get_nodestate(&self->tracker, input->origin->node) != jive_traversal_nodestate_behind)
			return false;
	}
	return true;
}

static void
jive_topdown_traverser_check_node(jive_full_traverser * self, jive_node * node)
{
	if (jive_traversal_tracker_get_nodestate(&self->tracker, node) != jive_traversal_nodestate_ahead)
		return;
	jive_traversal_tracker_set_nodestate(&self->tracker, node, jive_traversal_nodestate_frontier);
}

static jive_node *
jive_topdown_traverser_next(jive_traverser * self_)
{
	jive_full_traverser * self = (jive_full_traverser *) self_;
	jive_node * node = jive_traversal_tracker_pop_top(&self->tracker);
	if (!node)
		return NULL;
	jive_traversal_tracker_set_nodestate(&self->tracker, node, jive_traversal_nodestate_behind);
	size_t n;
	for (n = 0; n < node->noutputs; n++) {
		jive_output * output = node->outputs[n];
		jive_input * user;
		JIVE_LIST_ITERATE(output->users, user, output_users_list) {
			jive_topdown_traverser_check_node(self, user->node);
		}
	}
	return node;
}

static void
jive_topdown_traverser_node_create(void * closure, jive_node * node)
{
	jive_full_traverser * self = (jive_full_traverser *) closure;
	if (jive_topdown_traverser_predecessors_visited(self, node)) {
		jive_traversal_tracker_set_nodestate(&self->tracker, node, jive_traversal_nodestate_behind);
	} else {
		jive_traversal_tracker_set_nodestate(&self->tracker, node, jive_traversal_nodestate_frontier);
	}
}

static void
jive_topdown_traverser_input_change(void * closure, jive_input * input, jive_output * old_origin, jive_output * new_origin)
{
	jive_full_traverser * self = (jive_full_traverser *) closure;
	
	jive_traversal_nodestate state = jive_traversal_tracker_get_nodestate(&self->tracker, input->node);
	
	/* ignore nodes that have been traversed already, or that are already
	marked for later traversal */
	if (state != jive_traversal_nodestate_ahead)
		return;
	
	/* make sure node is visited eventually, might now be visited earlier
	as depth of the node could be lowered */
	jive_traversal_tracker_set_nodestate(&self->tracker, input->node, jive_traversal_nodestate_frontier);
}

const jive_traverser_class JIVE_TOPDOWN_TRAVERSER = {
	fini : &jive_full_traverser_fini,
	next : &jive_topdown_traverser_next,
};

static void
jive_topdown_traverser_init_top_nodes(jive_full_traverser * self, jive_region * region)
{
	jive_region * subregion;
	JIVE_LIST_ITERATE(region->subregions, subregion, region_subregions_list)
		jive_topdown_traverser_init_top_nodes(self, subregion);

	jive_node * node;
	JIVE_LIST_ITERATE(region->top_nodes, node, region_top_node_list)
		jive_traversal_tracker_set_nodestate(&self->tracker, node, jive_traversal_nodestate_frontier);
}

static void
jive_topdown_traverser_init(jive_full_traverser * self, jive_graph * graph)
{
	self->base.class_ = &JIVE_TOPDOWN_TRAVERSER;
	jive_full_traverser_init(self, graph);
	jive_topdown_traverser_init_top_nodes(self, graph->root_region);

	self->callbacks[self->ncallbacks ++] = jive_node_notifier_slot_connect(&graph->on_node_create,
		jive_topdown_traverser_node_create, self);
	self->callbacks[self->ncallbacks ++] = jive_input_change_notifier_slot_connect(
		&graph->on_input_change, jive_topdown_traverser_input_change, self);
}

jive_traverser *
jive_topdown_traverser_create(jive_graph * graph)
{
	jive_full_traverser * self = jive_context_malloc(graph->context, sizeof(*self));
	jive_topdown_traverser_init(self, graph);
	return &self->base;
}

static void
jive_bottomup_traverser_check_node(jive_full_traverser * self, jive_node * node)
{
	if (jive_traversal_tracker_get_nodestate(&self->tracker, node) != jive_traversal_nodestate_ahead)
		return;
	jive_traversal_tracker_set_nodestate(&self->tracker, node, jive_traversal_nodestate_frontier);
}

static jive_node *
jive_bottomup_traverser_next(jive_traverser * self_)
{
	jive_full_traverser * self = (jive_full_traverser *) self_;
	jive_node * node = jive_traversal_tracker_pop_bottom(&self->tracker);
	if (!node)
		return NULL;
	jive_traversal_tracker_set_nodestate(&self->tracker, node, jive_traversal_nodestate_behind);
	size_t n;
	for (n = 0; n < node->ninputs; n++) {
		jive_input * input = node->inputs[n];
		jive_bottomup_traverser_check_node(self, input->origin->node);
	}
	return node;
}

static void
jive_bottomup_traverser_node_create(void * closure, jive_node * node)
{
	jive_full_traverser * self = (jive_full_traverser *) closure;
	jive_traversal_tracker_set_nodestate(&self->tracker, node, jive_traversal_nodestate_behind);
}

static void
jive_bottomup_revisit_traverser_node_create(void * closure, jive_node * node)
{
	jive_full_traverser * self = (jive_full_traverser *) closure;
	jive_traversal_tracker_set_nodestate(&self->tracker, node, jive_traversal_nodestate_frontier);
}

static void
jive_bottomup_traverser_node_destroy(void * closure, jive_node * node)
{
	jive_full_traverser * self = (jive_full_traverser *) closure;
	size_t n;
	for (n = 0; n < node->ninputs; n++) {
		jive_input * input = node->inputs[n];
		jive_bottomup_traverser_check_node(self, input->origin->node);
	}
}

static void
jive_bottomup_traverser_input_change(void * closure, jive_input * input, jive_output * old_origin, jive_output * new_origin)
{
	jive_full_traverser * self = (jive_full_traverser *) closure;
	
	jive_traversal_nodestate state = jive_traversal_tracker_get_nodestate(&self->tracker, old_origin->node);
	
	/* ignore nodes that have been traversed already, or that are already
	marked for later traversal */
	if (state != jive_traversal_nodestate_ahead)
		return;
	
	/* make sure node is visited eventually, might now be visited earlier
	as there (potentially) is one less obstructing node below */
	jive_traversal_tracker_set_nodestate(&self->tracker, old_origin->node, jive_traversal_nodestate_frontier);
}

const jive_traverser_class JIVE_BOTTOMUP_TRAVERSER = {
	fini : &jive_full_traverser_fini,
	next : &jive_bottomup_traverser_next,
};

static void
jive_bottomup_traverser_init(jive_full_traverser * self, jive_graph * graph)
{
	self->base.class_ = &JIVE_BOTTOMUP_TRAVERSER;
	jive_full_traverser_init(self, graph);
	jive_node * node;
	JIVE_LIST_ITERATE(graph->bottom, node, graph_bottom_list) {
		jive_traversal_tracker_set_nodestate(&self->tracker, node, jive_traversal_nodestate_frontier);
	}
	self->callbacks[self->ncallbacks ++] = jive_node_notifier_slot_connect(&graph->on_node_create, jive_bottomup_traverser_node_create, self);
	self->callbacks[self->ncallbacks ++] = jive_node_notifier_slot_connect(&graph->on_node_destroy, jive_bottomup_traverser_node_destroy, self);
	self->callbacks[self->ncallbacks ++] = jive_input_change_notifier_slot_connect(&graph->on_input_change, jive_bottomup_traverser_input_change, self);
}

static void
jive_bottomup_revisit_traverser_init(jive_full_traverser * self, jive_graph * graph)
{
	self->base.class_ = &JIVE_BOTTOMUP_TRAVERSER;
	jive_full_traverser_init(self, graph);
	jive_node * node;
	JIVE_LIST_ITERATE(graph->bottom, node, graph_bottom_list) {
		jive_traversal_tracker_set_nodestate(&self->tracker, node, jive_traversal_nodestate_frontier);
	}
	self->callbacks[self->ncallbacks ++] = jive_node_notifier_slot_connect(&graph->on_node_create, jive_bottomup_revisit_traverser_node_create, self);
	self->callbacks[self->ncallbacks ++] = jive_node_notifier_slot_connect(&graph->on_node_destroy, jive_bottomup_traverser_node_destroy, self);
	self->callbacks[self->ncallbacks ++] = jive_input_change_notifier_slot_connect(&graph->on_input_change, jive_bottomup_traverser_input_change, self);
}

jive_traverser *
jive_bottomup_traverser_create(jive_graph * graph)
{
	jive_full_traverser * self = jive_context_malloc(graph->context, sizeof(*self));
	jive_bottomup_traverser_init(self, graph);
	return &self->base;
}

jive_traverser *
jive_bottomup_revisit_traverser_create(jive_graph * graph)
{
	jive_full_traverser * self = jive_context_malloc(graph->context, sizeof(*self));
	jive_bottomup_revisit_traverser_init(self, graph);
	return &self->base;
}

/* cone traversers */

static void
jive_upward_cone_traverser_node_destroy(void * closure, jive_node * node)
{
	jive_full_traverser * self = (jive_full_traverser *) closure;
	
	jive_traversal_nodestate state = jive_traversal_tracker_get_nodestate(&self->tracker, node);
	
	if (state != jive_traversal_nodestate_frontier)
		return;
	
	size_t n;
	for (n = 0; n < node->ninputs; n++) {
		jive_input * input = node->inputs[n];
		jive_bottomup_traverser_check_node(self, input->origin->node);
	}
}

static void
jive_upward_cone_traverser_input_change(void * closure, jive_input * input, jive_output * old_origin, jive_output * new_origin)
{
	jive_full_traverser * self = (jive_full_traverser *) closure;
	
	/* for node of new origin, it may now belong to the cone */
	jive_traversal_nodestate state = jive_traversal_tracker_get_nodestate(&self->tracker, input->node);
	if (state != jive_traversal_nodestate_ahead) {
		state = jive_traversal_tracker_get_nodestate(&self->tracker, new_origin->node);
		if (state == jive_traversal_nodestate_ahead)
			jive_traversal_tracker_set_nodestate(&self->tracker, new_origin->node, jive_traversal_nodestate_frontier);
	}
	
	/* for node of old origin, it may cease to belong to the cone */
	state = jive_traversal_tracker_get_nodestate(&self->tracker, old_origin->node);
	if (state == jive_traversal_nodestate_frontier) {
		size_t n;
		for (n = 0; n < old_origin->node->noutputs; n++) {
			jive_output * output = old_origin->node->outputs[n];
			jive_input * user;
			JIVE_LIST_ITERATE(output->users, user, output_users_list) {
				if (user == input)
					continue;
				state = jive_traversal_tracker_get_nodestate(&self->tracker, user->node);
				if (state != jive_traversal_nodestate_ahead)
					return;
			}
		}
		jive_traversal_tracker_set_nodestate(&self->tracker, old_origin->node, jive_traversal_nodestate_ahead);
	}
}

const jive_traverser_class JIVE_UPWARD_CONE_TRAVERSER = {
	fini : &jive_full_traverser_fini,
	next : &jive_bottomup_traverser_next,
};

static void
jive_upward_cone_traverser_init(jive_full_traverser * self, jive_graph * graph, jive_node * node)
{
	self->base.class_ = &JIVE_UPWARD_CONE_TRAVERSER;
	jive_full_traverser_init(self, graph);
	
	jive_traversal_tracker_set_nodestate(&self->tracker, node, jive_traversal_nodestate_frontier);
	
	/* self->callbacks[self->ncallbacks ++] = jive_node_notifier_slot_connect(&graph->on_node_create, jive_bottomup_traverser_node_create, self); */
	self->callbacks[self->ncallbacks ++] = jive_node_notifier_slot_connect(&graph->on_node_destroy, jive_upward_cone_traverser_node_destroy, self);
	self->callbacks[self->ncallbacks ++] = jive_input_change_notifier_slot_connect(&graph->on_input_change, jive_upward_cone_traverser_input_change, self);
}

jive_traverser *
jive_upward_cone_traverser_create(jive_node * node)
{
	jive_graph * graph = node->region->graph;
	jive_full_traverser * self = jive_context_malloc(graph->context, sizeof(*self));
	jive_upward_cone_traverser_init(self, graph, node);
	return &self->base;
}

typedef struct jive_bottomup_slave_traverser jive_bottomup_slave_traverser;
typedef struct jive_region_traverser_hash jive_region_traverser_hash;
JIVE_DECLARE_HASH_TYPE(jive_region_traverser_hash, jive_bottomup_slave_traverser, const jive_region *, region, hash_chain);

struct jive_bottomup_region_traverser {
	jive_graph * graph;
	jive_region_traverser_hash region_hash;
	
	jive_tracker_slot slot;
	
	jive_tracker_depth_state * behind_state;
};

struct jive_bottomup_slave_traverser {
	jive_traverser base;
	jive_bottomup_region_traverser * master;
	jive_tracker_depth_state * frontier_state;
	const jive_region * region;
	struct {
		jive_bottomup_slave_traverser * prev;
		jive_bottomup_slave_traverser * next;
	} hash_chain;
};

JIVE_DEFINE_HASH_TYPE(jive_region_traverser_hash, jive_bottomup_slave_traverser, const jive_region *, region, hash_chain);

static void
jive_bottomup_region_traverser_check_above(jive_bottomup_region_traverser * self, jive_node * node);

static jive_node *
jive_bottomup_slave_traverser_next(jive_traverser * self_)
{
	jive_bottomup_slave_traverser * self = (jive_bottomup_slave_traverser *) self_;
	
	jive_tracker_nodestate * nodestate = jive_tracker_depth_state_pop_bottom(self->frontier_state);
	if (!nodestate)
		return NULL;
	
	jive_node * node = nodestate->node;
	
	nodestate->state = 1;
	jive_tracker_depth_state_add(self->master->behind_state, nodestate, node->depth_from_root);
	jive_bottomup_region_traverser_check_above(self->master, node);
	
	return nodestate->node;
}

const jive_traverser_class JIVE_BOTTOMUP_SLAVE_TRAVERSER = {
	fini : 0,
	next : jive_bottomup_slave_traverser_next,
};

static jive_bottomup_slave_traverser *
jive_bottomup_slave_traverser_create(jive_bottomup_region_traverser * master, const jive_region * region)
{
	jive_graph * graph = master->graph;
	jive_context * context = graph->context;
	
	jive_bottomup_slave_traverser * self = jive_context_malloc(context, sizeof(*self));
	
	self->base.graph = graph;
	self->base.class_ = &JIVE_BOTTOMUP_SLAVE_TRAVERSER;
	self->master = master;
	self->frontier_state = jive_graph_reserve_tracker_depth_state(graph);
	self->region = region;
	jive_region_traverser_hash_insert(&master->region_hash, self);
	
	return self;
}

static void
jive_bottomup_slave_traverser_destroy(jive_bottomup_slave_traverser * self)
{
	jive_graph_return_tracker_depth_state(self->base.graph, self->frontier_state);
	jive_region_traverser_hash_remove(&self->master->region_hash, self);
	jive_context_free(self->base.graph->context, self);
}

static jive_bottomup_slave_traverser *
jive_bottomup_region_traverser_map_region(jive_bottomup_region_traverser * self, const jive_region * region)
{
	jive_bottomup_slave_traverser * slave;
	slave = jive_region_traverser_hash_lookup(&self->region_hash, region);
	if (slave)
		return slave;
	
	return jive_bottomup_slave_traverser_create(self, region);
}

static jive_tracker_nodestate *
jive_bottomup_region_traverser_map_node(jive_bottomup_region_traverser * self, jive_node * node)
{
	return jive_node_get_tracker_state(node, self->slot);
}

static void
jive_bottomup_region_traverser_check_above(jive_bottomup_region_traverser * self, jive_node * node)
{
	size_t n;
	for (n = 0; n < node->ninputs; n++) {
		jive_input * input = node->inputs[n];
		
		jive_node * above = input->origin->node;
		jive_tracker_nodestate * nodestate = jive_bottomup_region_traverser_map_node(self, above);
		if (nodestate->state != jive_tracker_nodestate_none)
			continue;
		
		jive_region * region = above->region;
		
		jive_bottomup_slave_traverser * slave = jive_bottomup_region_traverser_map_region(self, region);
		nodestate->state = 0;
		jive_tracker_depth_state_add(slave->frontier_state, nodestate, above->depth_from_root);
	}
}

void
jive_bottomup_region_traverser_pass(jive_bottomup_region_traverser * self, jive_node * node)
{
	jive_tracker_nodestate * nodestate = jive_bottomup_region_traverser_map_node(self, node);
	JIVE_DEBUG_ASSERT(nodestate->state == 0);
	
	jive_bottomup_slave_traverser * slave = jive_bottomup_region_traverser_map_region(self, node->region);
	
	jive_tracker_depth_state_remove(slave->frontier_state, nodestate, node->depth_from_root);
	nodestate->state = 1;
	jive_tracker_depth_state_add(self->behind_state, nodestate, node->depth_from_root);
	
	jive_bottomup_region_traverser_check_above(self, node);
}

jive_traverser *
jive_bottomup_region_traverser_get_node_traverser(jive_bottomup_region_traverser * self, jive_region * region)
{
	jive_bottomup_slave_traverser * slave = jive_bottomup_region_traverser_map_region(self, region);
	return &slave->base;
}

jive_bottomup_region_traverser *
jive_bottomup_region_traverser_create(jive_graph * graph)
{
	jive_context * context = graph->context;
	jive_bottomup_region_traverser * self = jive_context_malloc(context, sizeof(*self));
	
	self->graph = graph;
	jive_region_traverser_hash_init(&self->region_hash, context);
	
	self->slot = jive_graph_reserve_tracker_slot(graph);
	self->behind_state = jive_graph_reserve_tracker_depth_state(graph);
	
	/* seed bottom nodes in root region */
	jive_bottomup_slave_traverser * root_slave = jive_bottomup_region_traverser_map_region(self, graph->root_region);
	jive_node * node;
	JIVE_LIST_ITERATE(graph->bottom, node, graph_bottom_list) {
		if (node->region != graph->root_region)
			continue;
		
		jive_tracker_nodestate * nodestate = jive_bottomup_region_traverser_map_node(self, node);
		nodestate->state = 0;
		jive_tracker_depth_state_add(root_slave->frontier_state, nodestate, node->depth_from_root);
	}
	
	return self;
}

void
jive_bottomup_region_traverser_destroy(jive_bottomup_region_traverser * self)
{
	jive_context * context = self->graph->context;
	
	struct jive_region_traverser_hash_iterator i = jive_region_traverser_hash_begin(&self->region_hash);
	while (i.entry) {
		jive_bottomup_slave_traverser * trav = i.entry;
		jive_region_traverser_hash_iterator_next(&i);
		
		jive_bottomup_slave_traverser_destroy(trav);
	}
	jive_region_traverser_hash_fini(&self->region_hash);
	
	jive_graph_return_tracker_slot(self->graph, self->slot);
	jive_graph_return_tracker_depth_state(self->graph, self->behind_state);
	
	jive_context_free(context, self);
}
