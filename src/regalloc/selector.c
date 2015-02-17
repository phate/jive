/*
 * Copyright 2010 2011 2012 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/regalloc/selector.h>

#include <jive/common.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/shaped-variable.h>
#include <jive/regalloc/xpoint-private.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/resource-private.h>
#include <jive/vsdg/statetype.h>
#include <jive/vsdg/variable.h>

#include <unordered_map>

jive_node_cost::jive_node_cost(jive_master_shaper_selector * master, jive_node * node)
	: state(jive_node_cost_state_ahead)
	, index((size_t) -1)
	, force_tree_root(false)
	, node_(node)
	, master_(master)
{
	jive_resource_class_count_init(&rescls_cost);
	jive_rescls_prio_array_init(&prio_array);
	prio_array.count[0] = jive_resource_class_priority_lowest;
	blocked_rescls_priority = jive_resource_class_priority_lowest;
}

jive_node_cost::~jive_node_cost()
{
	jive_resource_class_count_fini(&rescls_cost);
}


static int
jive_node_cost_prio_heap_cmp(const jive_node_cost * a, const jive_node_cost * b)
{
	return jive_rescls_prio_array_compare(&a->prio_array, &b->prio_array);
}

void
jive_node_cost_prio_heap_init(jive_node_cost_prio_heap * self)
{
	self->nitems = 0;
}

void
jive_node_cost_prio_heap_add(jive_node_cost_prio_heap * self, jive_node_cost * item)
{
	JIVE_DEBUG_ASSERT(item->state == jive_node_cost_state_queue);
	JIVE_DEBUG_ASSERT(item->index == (size_t) -1);
	size_t index = self->nitems;
	self->items.resize(self->items.size()+1);
	self->nitems ++;
	
	while (index) {
		size_t parent = (index - 1) >> 1;
		jive_node_cost * pitem = self->items[parent];
		if (jive_node_cost_prio_heap_cmp(pitem, item) < 0)
			break;
		
		self->items[index] = pitem;
		JIVE_DEBUG_ASSERT(pitem->state == jive_node_cost_state_queue);
		pitem->index = index;
		index = parent;
	}
	
	self->items[index] = item;
	item->index = index;
}

jive_node_cost *
jive_node_cost_prio_heap_peek(const jive_node_cost_prio_heap * self)
{
	if (self->nitems)
		return self->items[0];
	else
		return 0;
}

static void
jive_node_cost_prio_heap_rebalance(jive_node_cost_prio_heap * self, size_t index)
{
	for (;;) {
		size_t lowest = index;
		size_t left = (index << 1) + 1;
		size_t right = left + 1;
		
		if (left < self->nitems) {
			if (jive_node_cost_prio_heap_cmp(self->items[left], self->items[lowest]) < 0)
				lowest = left;
		}
		if (right < self->nitems) {
			if (jive_node_cost_prio_heap_cmp(self->items[right], self->items[lowest]) < 0)
				lowest = right;
		}
		
		if (lowest == index)
			break;
		
		jive_node_cost * a = self->items[index];
		jive_node_cost * b = self->items[lowest];
		
		self->items[index] = b;
		JIVE_DEBUG_ASSERT(b->state == jive_node_cost_state_queue);
		b->index = index;
		
		self->items[lowest] = a;
		JIVE_DEBUG_ASSERT(a->state == jive_node_cost_state_queue);
		a->index = lowest;
		
		index = lowest;
	}
}

void
jive_node_cost_prio_heap_remove(jive_node_cost_prio_heap * self, jive_node_cost * item)
{
	JIVE_DEBUG_ASSERT(item->state == jive_node_cost_state_queue);
	size_t index = item->index;
	jive_node_cost * tmp = self->items[self->nitems -1];
	while (index) {
		size_t parent = (index - 1) >> 1;
		jive_node_cost * pitem = self->items[parent];
		if (jive_node_cost_prio_heap_cmp(pitem, tmp) <= 0)
			break;
		self->items[index] = pitem;
		JIVE_DEBUG_ASSERT(pitem->state == jive_node_cost_state_queue);
		pitem->index = index;
		index = parent;
	}
	self->items[index] = tmp;
	tmp->index = index;
	
	jive_node_cost_prio_heap_rebalance(self, item->index);
	item->index = (size_t) -1;
	self->nitems --;
}


void
jive_node_cost_stack_init(jive_node_cost_stack * self)
{
	self->nitems = 0;
}

void
jive_node_cost_stack_add(jive_node_cost_stack * self, jive_node_cost * item)
{
	JIVE_DEBUG_ASSERT(item->state == jive_node_cost_state_stack);
	JIVE_DEBUG_ASSERT(item->index == (size_t) -1);
	self->items.resize(self->items.size()+1);
	self->items[self->nitems] = item;
	item->index = self->nitems;
	self->nitems ++;
}

void
jive_node_cost_stack_remove(jive_node_cost_stack * self, jive_node_cost * item)
{
	JIVE_DEBUG_ASSERT(item->state == jive_node_cost_state_stack);
	size_t index = item->index;
	JIVE_DEBUG_ASSERT(index < self->nitems);
	JIVE_DEBUG_ASSERT(self->items[index] == item);
	
	item->index = (size_t) -1;
	self->nitems --;
	while (index < self->nitems) {
		self->items[index] = self->items[index + 1];
		self->items[index]->index = index;
		index ++;
	}
}

jive_node_cost *
jive_node_cost_stack_peek(const jive_node_cost_stack * self)
{
	if (self->nitems)
		return self->items[self->nitems - 1];
	else
		return NULL;
}

static void
push_node_stack(jive_region_shaper_selector * self, jive_node_cost * node_cost)
{
	JIVE_DEBUG_ASSERT(self->region == node_cost->node()->region);
	switch (node_cost->state) {
		case jive_node_cost_state_ahead:
			break;
		case jive_node_cost_state_queue:
			jive_node_cost_prio_heap_remove(&self->prio_heap, node_cost);
			break;
		case jive_node_cost_state_stack:
			return;
		case jive_node_cost_state_done:
			return;
	}
	
	node_cost->state = jive_node_cost_state_stack;
	jive_node_cost_stack_add(&self->node_stack, node_cost);
}

void
jive_region_shaper_selector_push_node_stack(jive_region_shaper_selector * self,
	struct jive_node * node)
{
	jive_node_cost * node_cost = self->master->map_node_internal(node);
	push_node_stack(self, node_cost);
}

jive_node *
jive_region_shaper_selector_select_node(jive_region_shaper_selector * self)
{
	if (self->node_stack.nitems) {
		return self->node_stack.items[self->node_stack.nitems - 1]->node();
	}
	
	if (self->prio_heap.nitems) {
		self->master->revalidate();
		jive_node_cost * node_cost = jive_node_cost_prio_heap_peek(&self->prio_heap);
		push_node_stack(self, node_cost);
		return node_cost->node();
	}
	
	return NULL;
}

typedef struct jive_ssavar_spill_position jive_ssavar_spill_position;
struct jive_ssavar_spill_position {
	jive_ssavar * ssavar;
	ssize_t position;
};

static void
sort_nodes(jive_region_shaper_selector * self, jive_node_cost * sorted_nodes[])
{
	size_t nsorted_nodes = 0, n;
	
	/* first add all outputs produced by stacked nodes */
	size_t k = self->node_stack.nitems;
	while (k) {
		k --;
		sorted_nodes[nsorted_nodes++] = self->node_stack.items[k];
	}
	
	size_t nprio_items = self->prio_heap.nitems;
	jive_node_cost * saved_heap[nprio_items];
	
	/* use prio heap to create sorted list */
	for (n = 0; n < nprio_items; n++)
		saved_heap[n] = self->prio_heap.items[n];
	
	for (n = 0; n < nprio_items; n++) {
		jive_node_cost * node_cost = jive_node_cost_prio_heap_peek(&self->prio_heap);
		sorted_nodes[nsorted_nodes++] = node_cost;
		jive_node_cost_prio_heap_remove(&self->prio_heap, node_cost);
	}
	
	/* restore prio heap afterwards */
	for (n = 0; n < nprio_items; n++) {
		self->prio_heap.items[n] = saved_heap[n];
		saved_heap[n]->index = n;
	}
	self->prio_heap.nitems = nprio_items;
}

static void
sort_ssavars(jive_region_shaper_selector * self, jive_ssavar * sorted_ssavars[])
{
	size_t nsorted_ssavars = 0;
	
	size_t n, k;
	
	size_t nsorted_nodes = self->prio_heap.nitems + self->node_stack.nitems;
	jive_node_cost * sorted_nodes[nsorted_nodes];
	sort_nodes(self, sorted_nodes);
	
	std::unordered_map<jive_ssavar*, jive_ssavar_spill_position> ssavar_pos_map;
	
	/* mark all (remaining) ssavars to be considered */
	jive_cutvar_xpoint * xpoint;
	JIVE_LIST_ITERATE(self->shaped_region->active_top.base.xpoints, xpoint, varcut_xpoints_list) {
		jive_ssavar_spill_position ssavar_pos;
		ssavar_pos.ssavar = xpoint->shaped_ssavar->ssavar;
		ssavar_pos.position = -1;
		ssavar_pos_map[ssavar_pos.ssavar] = ssavar_pos;
	}
	
	/* FIXME: mix in imported origins once they are tracked as well */
	
	/* add ssavars produced by nodes to be scheduled soon in priority order */
	for (k = 0; k < nsorted_nodes; k++) {
		jive_node * node = sorted_nodes[k]->node();
		for (n = 0; n < node->noutputs; n++) {
			jive::output * output = node->outputs[n];
			jive_shaped_ssavar * shaped_ssavar;
			shaped_ssavar = jive_varcut_map_output(&self->shaped_region->active_top.base, output);
			if (!shaped_ssavar)
				continue;
			sorted_ssavars[nsorted_ssavars ++] = shaped_ssavar->ssavar;
			jive_ssavar_spill_position ssavar_pos = ssavar_pos_map[shaped_ssavar->ssavar];
			ssavar_pos_map.erase(ssavar_pos.ssavar);
		}
	}
	
	/* now add all remaining ssavars */
	for (auto i = ssavar_pos_map.begin(); i != ssavar_pos_map.end();) {
		jive_ssavar_spill_position ssavar_pos = i->second;
		sorted_ssavars[nsorted_ssavars ++] = ssavar_pos.ssavar;
		i = ssavar_pos_map.erase(i);
	}
}

jive_ssavar *
jive_region_shaper_selector_select_spill(jive_region_shaper_selector * self,
	const jive_resource_class * rescls,
	jive_node * disallow_origins)
{
	size_t nsorted_ssavars = self->shaped_region->active_top.base.ssavar_map.size();
	jive_ssavar * sorted_ssavars[nsorted_ssavars];
	sort_ssavars(self, sorted_ssavars);
	
	size_t k = nsorted_ssavars;
	while (k) {
		k --;
		jive_ssavar * ssavar = sorted_ssavars[k];
		if (!jive_variable_may_spill(ssavar->variable))
			continue;
		
		if (ssavar->origin->node() == disallow_origins)
			continue;
		
		const jive_resource_class * var_rescls = jive_variable_get_resource_class(ssavar->variable);
		if (jive_resource_class_intersection(rescls, var_rescls) != var_rescls)
			continue;
		
		return ssavar;
	}
	
	JIVE_DEBUG_ASSERT(false);
	return 0;
}

static inline jive_resource_class_priority
jive_rescls_priority_min(jive_resource_class_priority a, jive_resource_class_priority b)
{
	return (a < b) ? a : b;
}

static void
compute_prio_value(jive_node_cost * node_cost)
{
	jive_rescls_prio_array_compute(&node_cost->prio_array, &node_cost->rescls_cost);
	node_cost->prio_array.count[0] = (size_t) node_cost->blocked_rescls_priority;
}

jive_master_shaper_selector::~jive_master_shaper_selector()
{
	for (jive_notifier * notifier : callbacks_) {
		jive_notifier_disconnect(notifier);
	}

	jive_computation_tracker_fini(&cost_computation_state_tracker_);
	
}

jive_master_shaper_selector::jive_master_shaper_selector(jive_shaped_graph * shaped_graph)
	: shaped_graph_(shaped_graph)
{
	jive_graph * graph = shaped_graph->graph;
	
	jive_computation_tracker_init(&cost_computation_state_tracker_, graph);
	
	init_region_recursive(graph->root_region);
	jive_node * node;
	JIVE_LIST_ITERATE(graph->bottom, node, graph_bottom_list) {
		try_add_frontier(node);
	}
	
	callbacks_.push_back(jive_node_notifier_slot_connect(
		&graph->on_node_create,
		jive_master_shaper_selector::node_create,
		this));
	callbacks_.push_back(jive_input_change_notifier_slot_connect(
		&graph->on_input_change,
		jive_master_shaper_selector::input_change,
		this));
	callbacks_.push_back(jive_node_notifier_slot_connect(
		&shaped_graph->on_shaped_node_create,
		jive_master_shaper_selector::shaped_node_create,
		this));
	callbacks_.push_back(jive_shaped_region_ssavar_notifier_slot_connect(
		&shaped_graph->on_shaped_region_ssavar_add,
		jive_master_shaper_selector::shaped_region_ssavar_add,
		this));
	callbacks_.push_back(jive_shaped_region_ssavar_notifier_slot_connect(
		&shaped_graph->on_shaped_region_ssavar_remove,
		jive_master_shaper_selector::shaped_region_ssavar_remove,
		this));
}

jive_region_shaper_selector *
jive_master_shaper_selector::map_region(const jive_region * region)
{
	jive_region_shaper_selector * region_selector = nullptr;
	auto i = region_map_.find(region);
	if (i != region_map_.end())
		region_selector = i.ptr();

	if (!region_selector) {
		jive_shaped_region * shaped_region = jive_shaped_graph_map_region(shaped_graph_, region);
		region_selector = region_shaper_selector_create(region, shaped_region);
	}
	return region_selector;
}

jive_node_cost *
jive_master_shaper_selector::map_node(jive_node * node)
{
	revalidate();
	return map_node_internal(node);
}

bool
jive_master_shaper_selector::check_node_selectable(const jive_node * node)
{
	size_t n;
	for (n = 0; n < node->noutputs; n++) {
		jive::output * output = node->outputs[n];
		jive::input * user;
		JIVE_LIST_ITERATE(output->users, user, output_users_list) {
			jive_node_cost * node_cost = map_node_internal(user->node);
			if (node_cost->state != jive_node_cost_state_done)
				return false;
		}
	}
	return true;
}

jive_node_cost *
jive_master_shaper_selector::map_node_internal(jive_node * node)
{
	auto i = node_map_.find(node);
	if (i != node_map_.end()) {
		return i.ptr();
	} else {
		return node_cost_create(node);
	}
}

void
jive_master_shaper_selector::invalidate_node(jive_node * node)
{
	jive_computation_tracker_invalidate(&cost_computation_state_tracker_, node);
}

void
jive_master_shaper_selector::revalidate_node(jive_node * node)
{
	jive_node_cost * node_cost;
	node_cost = map_node_internal(node);
	
	jive_resource_class_count cost;
	jive_resource_class_count_init(&cost);
	compute_node_cost(&cost, node);
	bool force_tree_root = !maybe_inner_node(node);
	jive_resource_class_priority blocked_rescls_priority;
	blocked_rescls_priority = compute_blocked_rescls_priority(node);
	
	bool changes =
		(force_tree_root != node_cost->force_tree_root) ||
		(blocked_rescls_priority != node_cost->blocked_rescls_priority) ||
		(!jive_resource_class_count_equals(&cost, &node_cost->rescls_cost));
	
	if (changes) {
		jive_region_shaper_selector * region_shaper;
		region_shaper = map_region(node->region);
		
		if (node_cost->state == jive_node_cost_state_queue)
			jive_node_cost_prio_heap_remove(&region_shaper->prio_heap, node_cost);
			
		node_cost->force_tree_root = force_tree_root;
		node_cost->blocked_rescls_priority = blocked_rescls_priority;
		jive_resource_class_count_copy(&node_cost->rescls_cost, &cost);
		compute_prio_value(node_cost);
		
		if (node_cost->state == jive_node_cost_state_queue)
			jive_node_cost_prio_heap_add(&region_shaper->prio_heap, node_cost);
			
		jive_computation_tracker_invalidate_below(&cost_computation_state_tracker_, node);
	}
	
	jive_resource_class_count_fini(&cost);
}

void
jive_master_shaper_selector::revalidate()
{
	jive_node * node;
	node = jive_computation_tracker_pop_top(&cost_computation_state_tracker_);
	while (node) {
		revalidate_node(node);
		node = jive_computation_tracker_pop_top(&cost_computation_state_tracker_);
	}
}

bool
jive_master_shaper_selector::assumed_active(
	const jive::output * output, const jive_region * region) const noexcept
{
	jive_shaped_region * shaped_region = jive_shaped_graph_map_region(shaped_graph_, region);
	return jive_region_varcut_output_is_active(&shaped_region->active_top, output);
}

bool
jive_master_shaper_selector::maybe_inner_node(const jive_node * node) const noexcept
{
	if (jive_shaped_graph_map_node(shaped_graph_, node))
		return false;
	
	size_t user_count = 0;
	size_t nonstate_count = 0;
	bool other_region = false;
	size_t n;
	for (n = 0; n < node->noutputs; n++) {
		jive::output * output = node->outputs[n];
		
		size_t output_user_count = 0;
		bool other_region = false;
		jive::input * user;
		JIVE_LIST_ITERATE(output->users, user, output_users_list) {
			output_user_count ++;
			other_region = other_region || (user->node->region != node->region);
		}
		
		if (!output_user_count)
			continue;
		
		if (!dynamic_cast<const jive::state::type*>(&output->type()))
			nonstate_count ++;
		
		user_count += output_user_count;
	}
	
	return (user_count <= 1) && (nonstate_count >= 1) && (!other_region);
}

void
jive_master_shaper_selector::compute_node_cost(
	jive_resource_class_count * cost,
	jive_node * node)
{
	if (jive_shaped_graph_map_node(shaped_graph_, node)) {
		jive_resource_class_count_clear(cost);
		return;
	}
	
	jive_region * region = node->region;
	
	jive_resource_class_count output_cost, input_cost, self_cost, eval_cost, this_eval_cost;
	jive_resource_class_count_init(&output_cost);
	jive_resource_class_count_init(&input_cost);
	jive_resource_class_count_init(&self_cost);
	jive_resource_class_count_init(&eval_cost);
	jive_resource_class_count_init(&this_eval_cost);
	size_t n;
	
	/* compute cost of self-representation */
	for (n = 0; n < node->noutputs; n++) {
		const jive_resource_class * rescls;
		rescls = jive_resource_class_relax(node->outputs[n]->required_rescls);
		jive_resource_class_count_add(&output_cost, rescls);
	}
	
	/* consider cost of all inputs */
	for (n = 0; n < node->ninputs; n++) {
		jive::input * input = node->inputs[n];
		if (assumed_active(input->origin(), region))
			continue;
		const jive_resource_class * rescls = jive_resource_class_relax(input->required_rescls);
		jive_resource_class_count_add(&input_cost, rescls);
	}
	
	jive_resource_class_count_copy(&self_cost, &output_cost);
	jive_resource_class_count_update_union(&self_cost, &input_cost);
	
	bool first = true;
	for (n = 0; n < node->ninputs; n++) {
		jive::input * input = node->inputs[n];
		jive_node_cost * last_eval = map_node(input->producer());
		
		if (last_eval->force_tree_root)
			continue;
		jive_resource_class_count_copy(&this_eval_cost, &input_cost);
		if (!assumed_active(input->origin(), region)) {
			const jive_resource_class * rescls = jive_resource_class_relax(input->required_rescls);
			jive_resource_class_count_add(&this_eval_cost, rescls);
		}
		jive_resource_class_count_update_add(&this_eval_cost, &last_eval->rescls_cost);
		
		if (first)
			jive_resource_class_count_copy(&eval_cost, &this_eval_cost);
		else
			jive_resource_class_count_update_intersection(&eval_cost, &this_eval_cost);
		
		first = false;
	}
	
	if (!first)
		jive_resource_class_count_update_union(&self_cost, &eval_cost);
	
	/* remove cost for those that would be removed by picking this node */
	for (n = 0; n < node->noutputs; n++) {
		if (assumed_active(node->outputs[n], region)) {
			const jive_resource_class * rescls;
			rescls = jive_resource_class_relax(node->outputs[n]->required_rescls);
			jive_resource_class_count_sub(&self_cost, rescls);
		}
	}
	
	jive_resource_class_count_copy(cost, &self_cost);
	
	jive_resource_class_count_fini(&output_cost);
	jive_resource_class_count_fini(&input_cost);
	jive_resource_class_count_fini(&self_cost);
	jive_resource_class_count_fini(&eval_cost);
	jive_resource_class_count_fini(&this_eval_cost);
}

jive_resource_class_priority
jive_master_shaper_selector::compute_blocked_rescls_priority(jive_node * node)
{
	if (jive_shaped_graph_map_node(shaped_graph_, node))
		return jive_root_resource_class.priority;
	
	jive_region * region = node->region;
	jive_resource_class_priority blocked_rescls_priority = jive_root_resource_class.priority;
	
	size_t n;
	for (n = 0; n < node->noutputs; n++) {
		jive::output * output = node->outputs[n];
		if (assumed_active(output, region)) {
			const jive_resource_class * rescls = jive_resource_class_relax(output->required_rescls);
			blocked_rescls_priority = jive_rescls_priority_min(blocked_rescls_priority, rescls->priority);
		}
	}
	
	for (n = 0; n < node->ninputs; n++) {
		jive::input * input = node->inputs[n];
		jive_node_cost * upper = map_node(input->producer());
		
		const jive_resource_class * input_rescls = jive_resource_class_relax(input->required_rescls);
		
		if (upper->force_tree_root) {
			if (assumed_active(input->origin(), region)) {
				blocked_rescls_priority = jive_rescls_priority_min(blocked_rescls_priority,
					input_rescls->priority);
			}
		} else {
			blocked_rescls_priority = jive_rescls_priority_min(blocked_rescls_priority,
				upper->blocked_rescls_priority);
		}
	}
	
	return blocked_rescls_priority;
}

void
jive_master_shaper_selector::try_add_frontier(jive_node * node)
{
	if (!check_node_selectable(node)) {
		return;
	}
	jive_node_cost * node_cost = map_node_internal(node);
	if (node_cost->state != jive_node_cost_state_ahead) {
		return;
	}
	
	node_cost->state = jive_node_cost_state_queue;
	jive_region_shaper_selector * region_shaper;
	region_shaper = map_region(node->region);
	jive_node_cost_prio_heap_add(&region_shaper->prio_heap, node_cost);
}

bool
jive_master_shaper_selector::remove_frontier(jive_node * node)
{
	jive_node_cost * node_cost = map_node_internal(node);
	
	if (node_cost->state == jive_node_cost_state_stack) {
		return true;
	}
	
	if (node_cost->state != jive_node_cost_state_queue) {
		return false;
	}
	
	jive_region_shaper_selector * region_shaper;
	region_shaper = map_region(node->region);
	jive_node_cost_prio_heap_remove(&region_shaper->prio_heap, node_cost);
	
	node_cost->state = jive_node_cost_state_ahead;
	
	return false;
}

void
jive_master_shaper_selector::mark_shaped(jive_node * node)
{
	jive_node_cost * node_cost = map_node_internal(node);
	
	jive_region_shaper_selector * region_shaper = map_region(node->region);
	switch (node_cost->state) {
		case jive_node_cost_state_ahead:
			break;
		case jive_node_cost_state_queue:
			jive_node_cost_prio_heap_remove(&region_shaper->prio_heap, node_cost);
			break;
		case jive_node_cost_state_stack:
			jive_node_cost_stack_remove(&region_shaper->node_stack, node_cost);
			break;
		case jive_node_cost_state_done:
			break;
	}
	
	invalidate_node(node);
	node_cost->state = jive_node_cost_state_done;
	
	for (size_t n = 0; n < node->ninputs; n++) {
		jive::input * input = node->inputs[n];
		try_add_frontier(input->producer());
	}
}

void
jive_master_shaper_selector::init_region_recursive(jive_region * region)
{
	jive_node * node;
	JIVE_LIST_ITERATE(region->top_nodes, node, region_top_node_list) {
		invalidate_node(node);
	}
	jive_region * subregion;
	JIVE_LIST_ITERATE(region->subregions, subregion, region_subregions_list) {
		init_region_recursive(subregion);
	}
}

jive_node_cost *
jive_master_shaper_selector::node_cost_create(jive_node * node)
{
	std::unique_ptr<jive_node_cost> cost(new jive_node_cost(this, node));
	return node_map_.insert(std::move(cost)).ptr();
}

jive_region_shaper_selector *
jive_master_shaper_selector::region_shaper_selector_create(const jive_region * region,
	const jive_shaped_region * shaped_region)
{
	std::unique_ptr<jive_region_shaper_selector> self(new jive_region_shaper_selector);
	
	self->master = this;
	
	self->region = region;
	self->shaped_region = shaped_region;
	
	jive_node_cost_prio_heap_init(&self->prio_heap);
	jive_node_cost_stack_init(&self->node_stack);

	jive_region_shaper_selector * result = self.get();
	region_map_.insert(std::move(self));

	return result;
}

/* callback closures */

void
jive_master_shaper_selector::shaped_node_create(void * closure, jive_node * node)
{
	jive_master_shaper_selector * self = (jive_master_shaper_selector *) closure;
	self->mark_shaped(node);
}

void
jive_master_shaper_selector::shaped_region_ssavar_add(
	void * closure, jive_shaped_region * shaped_region,
	jive_shaped_ssavar * shaped_ssavar)
{
	jive_master_shaper_selector * self = (jive_master_shaper_selector *) closure;
	self->invalidate_node(shaped_ssavar->ssavar->origin->node());
}

void
jive_master_shaper_selector::shaped_region_ssavar_remove(
	void * closure, jive_shaped_region * shaped_region,
	jive_shaped_ssavar * shaped_ssavar)
{
	jive_master_shaper_selector * self = (jive_master_shaper_selector *) closure;
	self->invalidate_node(shaped_ssavar->ssavar->origin->node());
}

void
jive_master_shaper_selector::node_create(void * closure, jive_node * node)
{
	jive_master_shaper_selector * self = (jive_master_shaper_selector *) closure;
	size_t n;
	bool stacked = false;
	for (n = 0; n < node->ninputs; n++) {
		jive::input * input = node->inputs[n];
		self->invalidate_node(input->producer());
		
		if (self->remove_frontier(input->producer())) {
			stacked = true;
		}
	}
	
	self->invalidate_node(node);
	self->try_add_frontier(node);
	
	/* if any of the inputs was on the priority stack, then this needs
	 * to go there as well */
	if (stacked) {
		jive_region_shaper_selector * region_shaper = self->map_region(node->region);
		jive_region_shaper_selector_push_node_stack(region_shaper, node);
	}
}

void
jive_master_shaper_selector::input_change(
	void * closure, jive::input * input, jive::output * old_origin,
	jive::output * new_origin)
{
	jive_master_shaper_selector * self = (jive_master_shaper_selector *) closure;
	
	self->try_add_frontier(old_origin->node());
	
	jive_node_cost * upper_node_cost, * lower_node_cost;
	upper_node_cost = self->map_node_internal(new_origin->node());
	lower_node_cost = self->map_node_internal(new_origin->node());
	
	if (lower_node_cost->state == jive_node_cost_state_ahead) {
		if (upper_node_cost->state == jive_node_cost_state_queue) {
			self->remove_frontier(new_origin->node());
		}
	}
	
	if (upper_node_cost->state == jive_node_cost_state_stack) {
		if (lower_node_cost->state != jive_node_cost_state_done) {
			jive_region_shaper_selector * region_shaper =
				self->map_region(input->node->region);
			jive_region_shaper_selector_push_node_stack(region_shaper, input->node);
		}
	}
}
