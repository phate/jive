/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_REGALLOC_SELECTOR_H
#define JIVE_REGALLOC_SELECTOR_H

#include <stdbool.h>
#include <stddef.h>

#include <jive/util/hash.h>
#include <jive/vsdg/resource.h>
#include <jive/vsdg/tracker.h>

#include <vector>

struct jive_context;
struct jive_node;
struct jive_region;
struct jive_resource_class;
struct jive_ssavar;
struct jive_shaped_graph;

typedef struct jive_region_shaper_selector jive_region_shaper_selector;
typedef struct jive_master_shaper_selector jive_master_shaper_selector;
typedef struct jive_node_cost jive_node_cost;
typedef struct jive_node_cost_hash jive_node_cost_hash;
typedef struct jive_node_cost_prio_heap jive_node_cost_prio_heap;
typedef struct jive_node_cost_stack jive_node_cost_stack;
typedef struct jive_region_shaper_selector_hash jive_region_shaper_selector_hash;

typedef enum {
	jive_node_cost_state_ahead = 0,
	jive_node_cost_state_queue = 1,
	jive_node_cost_state_stack = 2,
	jive_node_cost_state_done = 3
} jive_node_cost_state;

struct jive_node_cost {
	struct jive_node * node;
	jive_master_shaper_selector * master;
	
	jive_node_cost_state state; /* whether in prio queue or stack */
	size_t index; /* index within either stack or heap */
	
	/* "cost" of the node, in resource class counts and
	reduced to array of scalars */
	jive_resource_class_count rescls_cost;
	jive_rescls_prio_array prio_array;
	
	/* blocked resource class with highest priority */
	jive_resource_class_priority blocked_rescls_priority;
	bool force_tree_root;
	
	struct {
		jive_node_cost * prev;
		jive_node_cost * next;
	} hash_chain;
};

jive_node_cost *
jive_node_cost_create(jive_master_shaper_selector * master, struct jive_node * node);

void
jive_node_cost_destroy(jive_node_cost * self);

struct jive_node_cost_prio_heap {
	size_t nitems;
	std::vector<jive_node_cost*> items;
};

void
jive_node_cost_prio_heap_init(jive_node_cost_prio_heap * self, struct jive_context * context);

void
jive_node_cost_prio_heap_add(jive_node_cost_prio_heap * self, jive_node_cost * item);

jive_node_cost *
jive_node_cost_prio_heap_peek(const jive_node_cost_prio_heap * self);

void
jive_node_cost_prio_heap_remove(jive_node_cost_prio_heap * self, jive_node_cost * item);

JIVE_DECLARE_HASH_TYPE(jive_node_cost_hash, jive_node_cost, struct jive_node *, node, hash_chain);

struct jive_node_cost_stack {
	size_t nitems;
	std::vector<jive_node_cost*> items;
};

void
jive_node_cost_stack_init(jive_node_cost_stack * self, struct jive_context * context);

void
jive_node_cost_stack_add(jive_node_cost_stack * self, jive_node_cost * item);

void
jive_node_cost_stack_remove(jive_node_cost_stack * self, jive_node_cost * item);

jive_node_cost *
jive_node_cost_stack_peek(const jive_node_cost_stack * self);

struct jive_region_shaper_selector {
	jive_master_shaper_selector * master;
	
	const struct jive_region * region;
	const struct jive_shaped_region * shaped_region;
	
	jive_node_cost_prio_heap prio_heap;
	jive_node_cost_stack node_stack;
	
	struct {
		jive_region_shaper_selector * prev;
		jive_region_shaper_selector * next;
	} hash_chain;
};

jive_region_shaper_selector *
jive_region_shaper_selector_create(jive_master_shaper_selector * master, const struct jive_region * region, const struct jive_shaped_region * shaped_region);

void
jive_region_shaper_selector_destroy(jive_region_shaper_selector * self);

struct jive_node *
jive_region_shaper_selector_select_node(jive_region_shaper_selector * self);

struct jive_ssavar *
jive_region_shaper_selector_select_spill(jive_region_shaper_selector * self, const struct jive_resource_class * rescls, struct jive_node * disallow_origins);

void
jive_region_shaper_selector_push_node_stack(jive_region_shaper_selector * self, struct jive_node * node);

JIVE_DECLARE_HASH_TYPE(jive_region_shaper_selector_hash, jive_region_shaper_selector, const struct jive_region *, region, hash_chain);

struct jive_master_shaper_selector {
	struct jive_shaped_graph * shaped_graph;
	struct jive_context * context;
	
	jive_node_cost_hash node_map;
	jive_region_shaper_selector_hash region_map;
	jive_computation_tracker cost_computation_state_tracker;
	
	struct jive_notifier * callbacks[5];
};

jive_master_shaper_selector *
jive_master_shaper_selector_create(struct jive_shaped_graph * shaped_graph);

void
jive_master_shaper_selector_destroy(jive_master_shaper_selector * self);

jive_region_shaper_selector *
jive_master_shaper_selector_map_region(jive_master_shaper_selector * self, const struct jive_region * region);

jive_node_cost *
jive_master_shaper_selector_map_node_internal(jive_master_shaper_selector * self, struct jive_node * node);

jive_node_cost *
jive_master_shaper_selector_map_node(jive_master_shaper_selector * self, struct jive_node * node);

void
jive_master_shaper_selector_invalidate_node(jive_master_shaper_selector * self, struct jive_node * node);

void
jive_master_shaper_selector_revalidate_node(jive_master_shaper_selector * self, struct jive_node * node);

void
jive_master_shaper_selector_revalidate(jive_master_shaper_selector * self);

bool
jive_master_shaper_selector_check_node_selectable(jive_master_shaper_selector * self, struct jive_node * node);

#endif
