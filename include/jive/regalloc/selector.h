/*
 * Copyright 2010 2011 2012 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_REGALLOC_SELECTOR_H
#define JIVE_REGALLOC_SELECTOR_H

#include <stdbool.h>
#include <stddef.h>

#include <jive/util/intrusive-hash.h>
#include <jive/vsdg/resource.h>
#include <jive/vsdg/tracker.h>

#include <vector>

namespace jive {
class input;
class output;
}

struct jive_node;
struct jive_region;
struct jive_resource_class;
struct jive_shaped_graph;
struct jive_shaped_ssavar;
struct jive_ssavar;

typedef struct jive_master_shaper_selector jive_master_shaper_selector;
typedef struct jive_node_cost jive_node_cost;
typedef struct jive_node_cost_prio_heap jive_node_cost_prio_heap;
typedef struct jive_node_cost_stack jive_node_cost_stack;
typedef struct jive_region_shaper_selector jive_region_shaper_selector;

typedef enum {
	jive_node_cost_state_ahead = 0,
	jive_node_cost_state_queue = 1,
	jive_node_cost_state_stack = 2,
	jive_node_cost_state_done = 3
} jive_node_cost_state;

class jive_node_cost {
public:
	~jive_node_cost();

	jive_node_cost(jive_master_shaper_selector * master, jive_node * node);
	
	inline jive_node * node() const noexcept { return node_; }

	jive_node_cost_state state; /* whether in prio queue or stack */
	size_t index; /* index within either stack or heap */
	
	/* "cost" of the node, in resource class counts and
	reduced to array of scalars */
	jive_resource_class_count rescls_cost;
	jive_rescls_prio_array prio_array;
	
	/* blocked resource class with highest priority */
	jive_resource_class_priority blocked_rescls_priority;
	bool force_tree_root;

private:
	jive_node * node_;
	jive_master_shaper_selector * master_;
	
	jive::detail::intrusive_hash_anchor<jive_node_cost> hash_chain;
public:
	typedef jive::detail::intrusive_hash_accessor<
		jive_node *,
		jive_node_cost,
		&jive_node_cost::node_,
		&jive_node_cost::hash_chain
	> hash_chain_accessor;
};

typedef jive::detail::owner_intrusive_hash<
	const jive_node *,
	jive_node_cost,
	jive_node_cost::hash_chain_accessor
> jive_node_cost_hash;

jive_node_cost *
jive_node_cost_create(jive_master_shaper_selector * master, struct jive_node * node);

struct jive_node_cost_prio_heap {
	size_t nitems;
	std::vector<jive_node_cost*> items;
};

void
jive_node_cost_prio_heap_init(jive_node_cost_prio_heap * self);

void
jive_node_cost_prio_heap_add(jive_node_cost_prio_heap * self, jive_node_cost * item);

jive_node_cost *
jive_node_cost_prio_heap_peek(const jive_node_cost_prio_heap * self);

void
jive_node_cost_prio_heap_remove(jive_node_cost_prio_heap * self, jive_node_cost * item);

struct jive_node_cost_stack {
	size_t nitems;
	std::vector<jive_node_cost*> items;
};

void
jive_node_cost_stack_init(jive_node_cost_stack * self);

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

private:
	jive::detail::intrusive_hash_anchor<jive_region_shaper_selector> hash_chain;
public:
	typedef jive::detail::intrusive_hash_accessor <
		const struct jive_region *,
		jive_region_shaper_selector,
		&jive_region_shaper_selector::region,
		&jive_region_shaper_selector::hash_chain
	> hash_chain_accessor;
};

typedef jive::detail::owner_intrusive_hash <
	const struct jive_region *,
	jive_region_shaper_selector,
	jive_region_shaper_selector::hash_chain_accessor
> jive_region_shaper_selector_hash;

jive_region_shaper_selector *
jive_region_shaper_selector_create(jive_master_shaper_selector * master,
	const struct jive_region * region, const struct jive_shaped_region * shaped_region);

struct jive_node *
jive_region_shaper_selector_select_node(jive_region_shaper_selector * self);

struct jive_ssavar *
jive_region_shaper_selector_select_spill(jive_region_shaper_selector * self,
	const struct jive_resource_class * rescls, struct jive_node * disallow_origins);

void
jive_region_shaper_selector_push_node_stack(jive_region_shaper_selector * self,
	struct jive_node * node);

class jive_master_shaper_selector {
public:
	~jive_master_shaper_selector();

	jive_master_shaper_selector(jive_shaped_graph * shaped_graph);

	inline jive_shaped_graph *
	shaped_graph() const noexcept { return shaped_graph_; }

	/* lookup or create shaper selector for region */
	jive_region_shaper_selector *
	map_region(const jive_region * region);

	/* lookup or create cost structure for node */
	jive_node_cost *
	map_node(jive_node * node);

	bool
	check_node_selectable(const jive_node * node);

	/* FIXME: all of the below should be private, but they are accessed
	 * in the unit tests */

	/* lookup or create cost structure for node, without validation */
	jive_node_cost *
	map_node_internal(jive_node * node);

	/* invalidate cost estimate for current node */
	void
	invalidate_node(jive_node * node);

	/* force recomputation of cost estimate for current node */
	void
	revalidate_node(jive_node * node);

	/* recompute cost estimates for all nodes */
	void
	revalidate();

private:
	/* determine whether output is assumed to be active at entry to region */
	bool
	assumed_active(const jive::output * output, const jive_region * region) const noexcept;

	/* test whether this node may be a non-root node of a subtree */
	bool
	maybe_inner_node(const jive_node * node) const noexcept;

	/* compute the resource class cost of the given node; store in "cost" */
	void
	compute_node_cost(
		jive_resource_class_count * cost,
		jive_node * node);

	/* compute priority for resource class of arguments/results of this node */
	jive_resource_class_priority
	compute_blocked_rescls_priority(jive_node * node);

	/* try to add this node to the frontier of selectable nodes; nothing
	will be done if node is not selectable yet */
	void
	try_add_frontier(jive_node * node);

	/* remove node from the frontier of selectable nodes; returns true if it
	was on the priority stack, and false otherwise */
	bool
	remove_frontier(jive_node * node);

	/* mark this node as shaped, we don't need to include it in our cost model anymory */
	void
	mark_shaped(jive_node * node);

	/* initialize computation state for nodes in region, recursively */
	void
	init_region_recursive(jive_region * region);

	/* create node_cost structure for the given node */
	jive_node_cost *
	node_cost_create(jive_node * node);

	/* create a selector for the given region */
	jive_region_shaper_selector *
	region_shaper_selector_create(
		const jive_region * region,
		const jive_shaped_region * shaped_region);

	static void
	shaped_node_create(
		jive_master_shaper_selector * closure, jive_node * node);
	static void
	shaped_region_ssavar_add(
		jive_master_shaper_selector * closure, jive_shaped_region * shaped_region,
		jive_shaped_ssavar * shaped_ssavar);
	static void
	shaped_region_ssavar_remove(
		jive_master_shaper_selector * closure, jive_shaped_region * shaped_region,
		jive_shaped_ssavar * shaped_ssavar);
	static void
	node_create(jive_master_shaper_selector * closure, jive_node * node);
	static void
	input_change(
		jive_master_shaper_selector * closure, jive::input * input, jive::output * old_origin,
		jive::output * new_origin);

	jive_shaped_graph * shaped_graph_;

	jive_node_cost_hash node_map_;
	jive_region_shaper_selector_hash region_map_;
	jive::computation_tracker cost_computation_state_tracker_;

	std::vector<jive::callback> callbacks_;
};

#endif
