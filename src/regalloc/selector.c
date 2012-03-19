#include <jive/regalloc/selector.h>

#include <jive/common.h>
#include <jive/context.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/shaped-variable.h>
#include <jive/regalloc/xpoint-private.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/resource-private.h>
#include <jive/vsdg/statetype.h>
#include <jive/vsdg/variable.h>

JIVE_DEFINE_HASH_TYPE(jive_node_cost_hash, jive_node_cost, jive_node *, node, hash_chain);
JIVE_DEFINE_HASH_TYPE(jive_region_shaper_selector_hash, jive_region_shaper_selector, const jive_region *, region, hash_chain);

jive_node_cost *
jive_node_cost_create(jive_master_shaper_selector * master, jive_node * node)
{
	jive_context * context = master->context;
	
	jive_node_cost * self = jive_context_malloc(context, sizeof(*self));
	self->node = node;
	self->master = master;
	self->state = jive_node_cost_state_ahead;
	self->index = (size_t) -1;
	
	jive_resource_class_count_init(&self->rescls_cost, context);
	jive_rescls_prio_array_init(&self->prio_array);
	self->prio_array.count[0] = jive_resource_class_priority_lowest;
	self->blocked_rescls_priority = jive_resource_class_priority_lowest;
	self->force_tree_root = false;
	
	jive_node_cost_hash_insert(&master->node_map, self);
	
	return self;
}

void
jive_node_cost_destroy(jive_node_cost * self)
{
	jive_context * context = self->master->context;
	
	jive_node_cost_hash_remove(&self->master->node_map, self);
	
	jive_resource_class_count_fini(&self->rescls_cost);
	jive_context_free(context, self);
}


static int
jive_node_cost_prio_heap_cmp(const jive_node_cost * a, const jive_node_cost * b)
{
	return jive_rescls_prio_array_compare(&a->prio_array, &b->prio_array);
}

void
jive_node_cost_prio_heap_init(jive_node_cost_prio_heap * self, jive_context * context)
{
	self->nitems = 0;
	self->space = 0;
	self->items = 0;
	self->context = context;
}

void
jive_node_cost_prio_heap_fini(jive_node_cost_prio_heap * self)
{
	jive_context_free(self->context, self->items);
}

static void
jive_node_cost_prio_heap_enlarge(jive_node_cost_prio_heap * self)
{
	if (self->nitems == self->space) {
		size_t new_space = self->space * 2 + 1;
		self->items = jive_context_realloc(self->context, self->items, new_space * sizeof(self->items[0]));
		size_t n;
		for (n = self->space; n < new_space; n++)
			self->items[n] = NULL;
		self->space = new_space;
	}
}

void
jive_node_cost_prio_heap_add(jive_node_cost_prio_heap * self, jive_node_cost * item)
{
	JIVE_DEBUG_ASSERT(item->state == jive_node_cost_state_queue);
	JIVE_DEBUG_ASSERT(item->index == (size_t) -1);
	size_t index = self->nitems;
	jive_node_cost_prio_heap_enlarge(self);
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
jive_node_cost_stack_init(jive_node_cost_stack * self, jive_context * context)
{
	self->nitems = self->space = 0;
	self->items = NULL;
	self->context = context;
}

void
jive_node_cost_stack_fini(jive_node_cost_stack * self)
{
	jive_context_free(self->context, self->items);
}

void
jive_node_cost_stack_add(jive_node_cost_stack * self, jive_node_cost * item)
{
	JIVE_DEBUG_ASSERT(item->state == jive_node_cost_state_stack);
	JIVE_DEBUG_ASSERT(item->index == (size_t) -1);
	if (self->nitems == self->space) {
		self->space = self->space * 2 + 1;
		self->items = jive_context_realloc(self->context, self->items, self->space * sizeof(self->items[0]));
	}
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
	JIVE_DEBUG_ASSERT(self->region == node_cost->node->region);
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
jive_region_shaper_selector_push_node_stack(jive_region_shaper_selector * self, struct jive_node * node)
{
	jive_node_cost * node_cost = jive_master_shaper_selector_map_node_internal(self->master, node);
	push_node_stack(self, node_cost);
}

jive_node *
jive_region_shaper_selector_select_node(jive_region_shaper_selector * self)
{
	if (self->node_stack.nitems)
		return self->node_stack.items[self->node_stack.nitems - 1]->node;
	
	if (self->prio_heap.nitems) {
		jive_master_shaper_selector_revalidate(self->master);
		jive_node_cost * node_cost = jive_node_cost_prio_heap_peek(&self->prio_heap);
		push_node_stack(self, node_cost);
		return node_cost->node;
	}
	
	return NULL;
}

jive_region_shaper_selector *
jive_region_shaper_selector_create(jive_master_shaper_selector * master, const jive_region * region, const jive_shaped_region * shaped_region)
{
	jive_context * context = master->context;
	
	jive_region_shaper_selector * self = jive_context_malloc(context, sizeof(*self));
	
	self->master = master;
	
	self->region = region;
	self->shaped_region = shaped_region;
	
	jive_node_cost_prio_heap_init(&self->prio_heap, context);
	jive_node_cost_stack_init(&self->node_stack, context);
	jive_region_shaper_selector_hash_insert(&master->region_map, self);
	
	return self;
}

void
jive_region_shaper_selector_destroy(jive_region_shaper_selector * self)
{
	jive_context * context = self->master->context;
	
	jive_node_cost_prio_heap_fini(&self->prio_heap);
	jive_node_cost_stack_fini(&self->node_stack);
	jive_region_shaper_selector_hash_remove(&self->master->region_map, self);
	
	jive_context_free(context, self);
}

void
jive_master_shaper_selector_invalidate_node(jive_master_shaper_selector * self, jive_node * node)
{
	jive_computation_tracker_invalidate(&self->cost_computation_state_tracker, node);
}

jive_region_shaper_selector *
jive_master_shaper_selector_map_region(jive_master_shaper_selector * self, const jive_region * region)
{
	jive_region_shaper_selector * region_selector;
	region_selector = jive_region_shaper_selector_hash_lookup(&self->region_map, region);
	if (!region_selector) {
		jive_shaped_region * shaped_region = jive_shaped_graph_map_region(self->shaped_graph, region);
		region_selector = jive_region_shaper_selector_create(self, region, shaped_region);
	}
	return region_selector;
}

jive_node_cost *
jive_master_shaper_selector_map_node_internal(jive_master_shaper_selector * self, jive_node * node)
{
	jive_node_cost * node_cost;
	node_cost = jive_node_cost_hash_lookup(&self->node_map, node);
	if (!node_cost)
		node_cost = jive_node_cost_create(self, node);
	return node_cost;
}

static bool
assumed_active(jive_master_shaper_selector * self, jive_output * output, jive_region * region)
{
	jive_shaped_region * shaped_region = jive_shaped_graph_map_region(self->shaped_graph, region);
	return jive_region_varcut_output_is_active(&shaped_region->active_top, output);
}

typedef struct jive_ssavar_spill_position jive_ssavar_spill_position;
struct jive_ssavar_spill_position {
	jive_ssavar * ssavar;
	ssize_t position;
	struct {
		jive_ssavar_spill_position * prev;
		jive_ssavar_spill_position * next;
	} hash_chain;
};

JIVE_DECLARE_HASH_TYPE(jive_ssavar_spill_position_hash, jive_ssavar_spill_position, jive_ssavar *, ssavar, hash_chain);
JIVE_DEFINE_HASH_TYPE(jive_ssavar_spill_position_hash, jive_ssavar_spill_position, jive_ssavar *, ssavar, hash_chain);
typedef struct jive_ssavar_spill_position_hash jive_ssavar_spill_position_hash;

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
	jive_context * context = self->master->context;
	
	size_t nsorted_nodes = self->prio_heap.nitems + self->node_stack.nitems;
	jive_node_cost * sorted_nodes[nsorted_nodes];
	sort_nodes(self, sorted_nodes);
	
	jive_ssavar_spill_position_hash ssavar_pos_map;
	jive_ssavar_spill_position_hash_init(&ssavar_pos_map, context);
	
	/* mark all (remaining) ssavars to be considered */
	jive_cutvar_xpoint * xpoint;
	JIVE_LIST_ITERATE(self->shaped_region->active_top.base.xpoints, xpoint, varcut_xpoints_list) {
		jive_ssavar_spill_position * ssavar_pos = jive_context_malloc(context, sizeof(*ssavar_pos));
		ssavar_pos->ssavar = xpoint->shaped_ssavar->ssavar;
		ssavar_pos->position = -1;
		jive_ssavar_spill_position_hash_insert(&ssavar_pos_map, ssavar_pos);
	}
	
	/* FIXME: mix in imported origins once they are tracked as well */
	
	/* add ssavars produced by nodes to be scheduled soon in priority order */
	for (k = 0; k < nsorted_nodes; k++) {
		jive_node * node = sorted_nodes[k]->node;
		for (n = 0; n < node->noutputs; n++) {
			jive_output * output = node->outputs[n];
			jive_shaped_ssavar * shaped_ssavar = jive_varcut_map_output(&self->shaped_region->active_top.base, output);
			if (!shaped_ssavar)
				continue;
			sorted_ssavars[nsorted_ssavars ++] = shaped_ssavar->ssavar;
			jive_ssavar_spill_position * ssavar_pos = jive_ssavar_spill_position_hash_lookup(&ssavar_pos_map, shaped_ssavar->ssavar);
			jive_ssavar_spill_position_hash_remove(&ssavar_pos_map, ssavar_pos);
			jive_context_free(context, ssavar_pos);
		}
	}
	
	/* now add all remaining ssavars */
	struct jive_ssavar_spill_position_hash_iterator i;
	i = jive_ssavar_spill_position_hash_begin(&ssavar_pos_map);
	while (i.entry) {
		jive_ssavar_spill_position * ssavar_pos = i.entry;
		jive_ssavar_spill_position_hash_iterator_next(&i);
		sorted_ssavars[nsorted_ssavars ++] = ssavar_pos->ssavar;
		jive_ssavar_spill_position_hash_remove(&ssavar_pos_map, ssavar_pos);
		jive_context_free(context, ssavar_pos);
	}
	
	jive_ssavar_spill_position_hash_fini(&ssavar_pos_map);
}

jive_ssavar *
jive_region_shaper_selector_select_spill(jive_region_shaper_selector * self, const jive_resource_class * rescls)
{
	size_t nsorted_ssavars = self->shaped_region->active_top.base.ssavar_map.nitems;
	jive_ssavar * sorted_ssavars[nsorted_ssavars];
	sort_ssavars(self, sorted_ssavars);
	
	size_t k = nsorted_ssavars;
	while (k) {
		k --;
		jive_ssavar * ssavar = sorted_ssavars[k];
		if (!jive_variable_may_spill(ssavar->variable))
			continue;
		
		const jive_resource_class * var_rescls = jive_variable_get_resource_class(ssavar->variable);
		if (jive_resource_class_intersection(rescls, var_rescls) != var_rescls)
			continue;
		
		return ssavar;
	}
	
	JIVE_DEBUG_ASSERT(false);
	return 0;
}


/* Test whether this node may be a non-root node of a subtree */
static bool
maybe_inner_node(jive_master_shaper_selector * self, jive_node * node)
{
	if (jive_shaped_graph_map_node(self->shaped_graph, node))
		return false;
	
	size_t user_count = 0;
	size_t nonstate_count = 0;
	bool other_region = false;
	size_t n;
	for (n = 0; n < node->noutputs; n++) {
		jive_output * output = node->outputs[n];
		
		size_t output_user_count = 0;
		bool other_region = false;
		jive_input * user;
		JIVE_LIST_ITERATE(output->users, user, output_users_list) {
			output_user_count ++;
			other_region = other_region || (user->node->region != node->region);
		}
		
		if (!output_user_count)
			continue;
		
		if (!jive_output_isinstance(output, &JIVE_STATE_OUTPUT))
			nonstate_count ++;
		
		user_count += output_user_count;
	}
	
	return (user_count <= 1) && (nonstate_count >= 1) && (!other_region);
}

static void
compute_node_cost(jive_master_shaper_selector * self, jive_resource_class_count * cost, jive_node * node)
{
	if (jive_shaped_graph_map_node(self->shaped_graph, node)) {
		jive_resource_class_count_clear(cost);
		return;
	}
	
	jive_region * region = node->region;
	jive_context * context = self->shaped_graph->context;
	
	jive_resource_class_count output_cost, input_cost, self_cost, eval_cost, this_eval_cost;
	jive_resource_class_count_init(&output_cost, context);
	jive_resource_class_count_init(&input_cost, context);
	jive_resource_class_count_init(&self_cost, context);
	jive_resource_class_count_init(&eval_cost, context);
	jive_resource_class_count_init(&this_eval_cost, context);
	size_t n;
	
	/* compute cost of self-representation */
	for (n = 0; n < node->noutputs; n++) {
		const jive_resource_class * rescls = jive_resource_class_relax(node->outputs[n]->required_rescls);
		jive_resource_class_count_add(&output_cost, rescls);
	}
	
	/* consider cost of all inputs */
	for (n = 0; n < node->ninputs; n++) {
		jive_input * input = node->inputs[n];
		if (assumed_active(self, input->origin, region))
			continue;
		const jive_resource_class * rescls = jive_resource_class_relax(input->required_rescls);
		jive_resource_class_count_add(&input_cost, rescls);
	}
	
	jive_resource_class_count_copy(&self_cost, &output_cost);
	jive_resource_class_count_update_union(&self_cost, &input_cost);
	
	bool first = true;
	for (n = 0; n < node->ninputs; n++) {
		jive_input * input = node->inputs[n];
		jive_node_cost * last_eval = jive_master_shaper_selector_map_node(self, input->origin->node);
		
		if (last_eval->force_tree_root)
			continue;
		jive_resource_class_count_copy(&this_eval_cost, &input_cost);
		if (!assumed_active(self, input->origin, region)) {
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
		if (assumed_active(self, node->outputs[n], region)) {
			const jive_resource_class * rescls = jive_resource_class_relax(node->outputs[n]->required_rescls);
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

static inline jive_resource_class_priority
jive_rescls_priority_min(jive_resource_class_priority a, jive_resource_class_priority b)
{
	return (a < b) ? a : b;
}

static jive_resource_class_priority
compute_blocked_rescls_priority(jive_master_shaper_selector * self, jive_node * node)
{
	if (jive_shaped_graph_map_node(self->shaped_graph, node))
		return jive_root_resource_class.priority;
	
	jive_region * region = node->region;
	jive_resource_class_priority blocked_rescls_priority = jive_root_resource_class.priority;
	
	size_t n;
	for (n = 0; n < node->noutputs; n++) {
		jive_output * output = node->outputs[n];
		if (assumed_active(self, output, region)) {
			const jive_resource_class * rescls = jive_resource_class_relax(output->required_rescls);
			blocked_rescls_priority = jive_rescls_priority_min(blocked_rescls_priority, rescls->priority);
		}
	}
	
	for (n = 0; n < node->ninputs; n++) {
		jive_input * input = node->inputs[n];
		jive_node_cost * upper = jive_master_shaper_selector_map_node(self, input->origin->node);
		
		const jive_resource_class * input_rescls = jive_resource_class_relax(input->required_rescls);
		
		if (upper->force_tree_root) {
			if (assumed_active(self, input->origin, region)) {
				blocked_rescls_priority = jive_rescls_priority_min(blocked_rescls_priority, input_rescls->priority);
			}
		} else {
			blocked_rescls_priority = jive_rescls_priority_min(blocked_rescls_priority, upper->blocked_rescls_priority);
		}
	}
	
	return blocked_rescls_priority;
}

void
compute_prio_value(jive_node_cost * node_cost)
{
	jive_rescls_prio_array_compute(&node_cost->prio_array, &node_cost->rescls_cost);
	node_cost->prio_array.count[0] = (size_t) node_cost->blocked_rescls_priority;
}

void
jive_master_shaper_selector_revalidate_node(jive_master_shaper_selector * self, jive_node * node)
{
	jive_node_cost * node_cost;
	node_cost = jive_master_shaper_selector_map_node_internal(self, node);
	
	jive_context * context = self->context;
	
	jive_resource_class_count cost;
	jive_resource_class_count_init(&cost, context);
	compute_node_cost(self, &cost, node);
	bool force_tree_root = !maybe_inner_node(self, node);
	jive_resource_class_priority blocked_rescls_priority = compute_blocked_rescls_priority(self, node);
	
	bool changes =
		(force_tree_root != node_cost->force_tree_root) ||
		(blocked_rescls_priority != node_cost->blocked_rescls_priority) ||
		(!jive_resource_class_count_equals(&cost, &node_cost->rescls_cost));
	
	if (changes) {
		jive_region_shaper_selector * region_shaper;
		region_shaper = jive_master_shaper_selector_map_region(self, node->region);
		
		if (node_cost->state == jive_node_cost_state_queue)
			jive_node_cost_prio_heap_remove(&region_shaper->prio_heap, node_cost);
			
		node_cost->force_tree_root = force_tree_root;
		node_cost->blocked_rescls_priority = blocked_rescls_priority;
		jive_resource_class_count_copy(&node_cost->rescls_cost, &cost);	
		compute_prio_value(node_cost);
		
		if (node_cost->state == jive_node_cost_state_queue)
			jive_node_cost_prio_heap_add(&region_shaper->prio_heap, node_cost);
			
		jive_computation_tracker_invalidate_below(&self->cost_computation_state_tracker, node);
	}
	
	jive_resource_class_count_fini(&cost);
}

void
jive_master_shaper_selector_revalidate(jive_master_shaper_selector * self)
{
	jive_node * node;
	node = jive_computation_tracker_pop_top(&self->cost_computation_state_tracker);
	while (node) {
		jive_master_shaper_selector_revalidate_node(self, node);
		node = jive_computation_tracker_pop_top(&self->cost_computation_state_tracker);
	}
}

bool
jive_master_shaper_selector_check_node_selectable(jive_master_shaper_selector * self, jive_node * node)
{
	size_t n;
	for (n = 0; n < node->noutputs; n++) {
		jive_output * output = node->outputs[n];
		jive_input * user;
		JIVE_LIST_ITERATE(output->users, user, output_users_list) {
			jive_node_cost * node_cost = jive_master_shaper_selector_map_node_internal(self, user->node);
			if (node_cost->state != jive_node_cost_state_done)
				return false;
		}
	}
	return true;
}

static void
jive_master_shaper_selector_try_add_frontier(jive_master_shaper_selector * self, jive_node * node)
{
	if (!jive_master_shaper_selector_check_node_selectable(self, node))
		return;
	jive_node_cost * node_cost = jive_master_shaper_selector_map_node_internal(self, node);
	if (node_cost->state != jive_node_cost_state_ahead)
		return;
	
	node_cost->state = jive_node_cost_state_queue;
	jive_region_shaper_selector * region_shaper;
	region_shaper = jive_master_shaper_selector_map_region(self, node->region);
	jive_node_cost_prio_heap_add(&region_shaper->prio_heap, node_cost);
}

static bool
jive_master_shaper_selector_remove_frontier(jive_master_shaper_selector * self, jive_node * node)
{
	jive_node_cost * node_cost = jive_master_shaper_selector_map_node_internal(self, node);
	
	if (node_cost->state == jive_node_cost_state_stack)
		return true;
	
	if (node_cost->state != jive_node_cost_state_queue)
		return false;
	
	jive_region_shaper_selector * region_shaper;
	region_shaper = jive_master_shaper_selector_map_region(self, node->region);
	jive_node_cost_prio_heap_remove(&region_shaper->prio_heap, node_cost);
	
	node_cost->state = jive_node_cost_state_ahead;
	
	return false;
}

static void
jive_master_shaper_selector_mark_shaped(jive_master_shaper_selector * self, jive_node * node)
{
	jive_node_cost * node_cost = jive_master_shaper_selector_map_node_internal(self, node);
	
	jive_region_shaper_selector * region_shaper = jive_master_shaper_selector_map_region(self, node->region);
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
	
	jive_master_shaper_selector_invalidate_node(self, node);
	node_cost->state = jive_node_cost_state_done;
	
	size_t n;
	for (n = 0; n < node->ninputs; n++) {
		jive_input * input = node->inputs[n];
		jive_master_shaper_selector_try_add_frontier(self, input->origin->node);
	}
}

jive_node_cost *
jive_master_shaper_selector_map_node(jive_master_shaper_selector * self, jive_node * node)
{
	jive_master_shaper_selector_revalidate(self);
	return jive_master_shaper_selector_map_node_internal(self, node);
}

static void
shaped_node_create(void * self_, jive_node * node)
{
	jive_master_shaper_selector * self = (jive_master_shaper_selector *) self_;
	jive_master_shaper_selector_mark_shaped(self, node);
}

static void
shaped_region_ssavar_add(void * self_, jive_shaped_region * shaped_region, jive_shaped_ssavar * shaped_ssavar)
{
	jive_master_shaper_selector * self = (jive_master_shaper_selector *) self_;
	jive_master_shaper_selector_invalidate_node(self, shaped_ssavar->ssavar->origin->node);
}

static void
shaped_region_ssavar_remove(void * self_, jive_shaped_region * shaped_region, jive_shaped_ssavar * shaped_ssavar)
{
	jive_master_shaper_selector * self = (jive_master_shaper_selector *) self_;
	jive_master_shaper_selector_invalidate_node(self, shaped_ssavar->ssavar->origin->node);
}

static void
node_create(void * self_, jive_node * node)
{
	jive_master_shaper_selector * self = (jive_master_shaper_selector *) self_;
	size_t n;
	bool stacked = false;
	for (n = 0; n < node->ninputs; n++) {
		jive_input * input = node->inputs[n];
		jive_master_shaper_selector_invalidate_node(self, input->origin->node);
		
		if (jive_master_shaper_selector_remove_frontier(self, input->origin->node))
			stacked = true;
	}
	
	jive_master_shaper_selector_invalidate_node(self, node);
	jive_master_shaper_selector_try_add_frontier(self, node);
	
	if (stacked) {
		jive_region_shaper_selector * region_shaper = jive_master_shaper_selector_map_region(self, node->region);
		jive_region_shaper_selector_push_node_stack(region_shaper, node);
	}
}

static void
input_change(void * self_, jive_input * input, jive_output * old_origin, jive_output * new_origin)
{
	jive_master_shaper_selector * self = (jive_master_shaper_selector *) self_;
	
	jive_master_shaper_selector_try_add_frontier(self, old_origin->node);
	
	jive_node_cost * upper_node_cost, * lower_node_cost;
	upper_node_cost = jive_master_shaper_selector_map_node_internal(self, new_origin->node);
	lower_node_cost = jive_master_shaper_selector_map_node_internal(self, new_origin->node);
	
	if (lower_node_cost->state == jive_node_cost_state_ahead) {
		if (upper_node_cost->state == jive_node_cost_state_queue)
			jive_master_shaper_selector_remove_frontier(self, new_origin->node);
	}
	
	if (upper_node_cost->state == jive_node_cost_state_stack) {
		if (lower_node_cost->state != jive_node_cost_state_done) {
			jive_region_shaper_selector * region_shaper = jive_master_shaper_selector_map_region(self, input->node->region);
			jive_region_shaper_selector_push_node_stack(region_shaper, input->node);
		}
	}
}

jive_master_shaper_selector *
jive_master_shaper_selector_create(jive_shaped_graph * shaped_graph)
{
	jive_graph * graph = shaped_graph->graph;
	jive_context * context = graph->context;
	
	jive_master_shaper_selector * self;
	self = jive_context_malloc(context, sizeof(*self));
	
	self->shaped_graph = shaped_graph;
	self->context = context;
	
	jive_node_cost_hash_init(&self->node_map, context);
	jive_region_shaper_selector_hash_init(&self->region_map, context);
	jive_computation_tracker_init(&self->cost_computation_state_tracker, graph);
	
	jive_node * node;
	JIVE_LIST_ITERATE(graph->top, node, graph_top_list) {
		jive_master_shaper_selector_invalidate_node(self, node);
	}
	
	JIVE_LIST_ITERATE(graph->bottom, node, graph_bottom_list) {
		jive_master_shaper_selector_try_add_frontier(self, node);
	}
	
	size_t n = 0;
	self->callbacks[n++] = jive_node_notifier_slot_connect(&graph->on_node_create, node_create, self);
	self->callbacks[n++] = jive_input_change_notifier_slot_connect(&graph->on_input_change, input_change, self);
	self->callbacks[n++] = jive_node_notifier_slot_connect(&shaped_graph->on_shaped_node_create, shaped_node_create, self);
	self->callbacks[n++] = jive_shaped_region_ssavar_notifier_slot_connect(&shaped_graph->on_shaped_region_ssavar_add, shaped_region_ssavar_add, self);
	self->callbacks[n++] = jive_shaped_region_ssavar_notifier_slot_connect(&shaped_graph->on_shaped_region_ssavar_remove, shaped_region_ssavar_remove, self);
	
	JIVE_DEBUG_ASSERT(n == sizeof(self->callbacks)/sizeof(self->callbacks[0]));
	
	return self;
}

void
jive_master_shaper_selector_destroy(jive_master_shaper_selector * self)
{
	jive_context * context = self->context;
	
	size_t n;
	for (n = 0; n < sizeof(self->callbacks)/sizeof(self->callbacks[0]); n++)
		jive_notifier_disconnect(self->callbacks[n]);
	
	struct jive_node_cost_hash_iterator i;
	i = jive_node_cost_hash_begin(&self->node_map);
	while (i.entry) {
		jive_node_cost * node_cost = i.entry;
		jive_node_cost_hash_iterator_next(&i);
		
		jive_node_cost_destroy(node_cost);
	}
	jive_node_cost_hash_fini(&self->node_map);
	
	struct jive_region_shaper_selector_hash_iterator j;
	j = jive_region_shaper_selector_hash_begin(&self->region_map);
	while (j.entry) {
		jive_region_shaper_selector * region_selector = j.entry;
		jive_region_shaper_selector_hash_iterator_next(&j);
		
		jive_region_shaper_selector_destroy(region_selector);
	}
	jive_region_shaper_selector_hash_fini(&self->region_map);
	
	jive_computation_tracker_fini(&self->cost_computation_state_tracker);
	
	jive_context_free(context, self);
}
