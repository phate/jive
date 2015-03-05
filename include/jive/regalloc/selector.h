/*
 * Copyright 2010 2011 2012 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_REGALLOC_SELECTOR_H
#define JIVE_REGALLOC_SELECTOR_H

#include <stdbool.h>
#include <stddef.h>

#include <set>
#include <unordered_set>
#include <vector>

#include <jive/util/intrusive-hash.h>
#include <jive/vsdg/resource.h>
#include <jive/vsdg/tracker.h>

struct jive_node;
struct jive_region;
struct jive_resource_class;
struct jive_shaped_graph;
struct jive_shaped_region;
struct jive_shaped_ssavar;
struct jive_ssavar;

namespace jive {
class input;
class output;

namespace regalloc {

class master_selector;

/* Structure used for tracking the selection priority of nodes, driven by
 * both "cost" and other criteria. */
class node_selection_order {
public:
	enum state_type {
		state_ahead = 0, /* not selectable */
		state_queue = 1, /* selectable, in queue according to cost */
		state_stack = 2, /* selectable, on priority stack */
		state_done = 3   /* scheduled, cost not tracked anymore */
	};

	/* compare priorities of nodes */
	class ptr_less {
	public:
		inline bool operator()(
			node_selection_order * x,
			node_selection_order * y) const noexcept
		{
			return jive_rescls_prio_array_compare(
				&x->prio_array(), &y->prio_array()) < 0;
		}
	};

	~node_selection_order();

	node_selection_order(master_selector * master, jive_node * node);
	
	inline jive_node * node() const noexcept { return node_; }
	
	/* some internals exposed read-only for unit tests */
	inline const jive_rescls_prio_array & prio_array() const noexcept { return prio_array_; }

	inline state_type state() const noexcept { return state_; }

	inline const std::multiset<node_selection_order *, ptr_less>::const_iterator
	queue_index() const noexcept { return queue_index_; }

	inline jive_resource_class_priority
	blocked_rescls_priority() const noexcept { return blocked_rescls_priority_; }

	inline bool force_tree_root() const noexcept { return force_tree_root_; }

	inline const jive_resource_class_count &
	rescls_cost() const noexcept { return rescls_cost_; }

private:
	/* compute the priority value for this node, based on resource
	 * usage counts */
	void
	compute_prio_value();

	/* whether in prio queue or stack */
	state_type state_;
	/* index within stack (only valid if state_stack) */
	size_t stack_index_;
	/* position in queue (only valid if state_queue) */
	std::multiset<node_selection_order *, ptr_less>::iterator queue_index_;

	/* "cost" of the node, in resource class counts and
	reduced to array of scalars */
	jive_resource_class_count rescls_cost_;
	jive_rescls_prio_array prio_array_;
	
	/* blocked resource class with highest priority */
	jive_resource_class_priority blocked_rescls_priority_;
	/* consider root of dependence tree for computation purposes */
	bool force_tree_root_;

	jive_node * node_;
	master_selector * master_;
	
	detail::intrusive_hash_anchor<node_selection_order> hash_chain;
public:
	typedef detail::intrusive_hash_accessor<
		jive_node *,
		node_selection_order,
		&node_selection_order::node_,
		&node_selection_order::hash_chain
	> hash_chain_accessor;

	friend class region_selector;
	friend class master_selector;
};

typedef detail::owner_intrusive_hash<
	const jive_node *,
	node_selection_order,
	node_selection_order::hash_chain_accessor
> node_selection_order_hash;

/* select next nodes to be shaped in region */
class region_selector {
public:
	region_selector(
		master_selector * master,
		const jive_region * region,
		const jive_shaped_region * shaped_region);

	/* select next node for shaping */
	jive_node *
	select_node();

	/* select variable suitable for spilling */
	jive_ssavar *
	select_spill(
		const jive_resource_class * rescls,
		jive_node * disallow_origins) const;

	/* push onto priority node selection stack -- this node will be
	 * preferred for selection over all other currently selectable nodes */
	void
	push_node_stack(
		jive_node * node);

	/* expose readable node queue (for unit tests) */
	inline const std::multiset<node_selection_order *, node_selection_order::ptr_less> &
	node_queue() const noexcept {
		return node_queue_;
	}

private:
	/* ssavars sorted by priority, highest priority ssavars first */
	std::vector<jive_ssavar *>
	prio_sorted_ssavars() const;

	/* lookup or create cost structure for node */
	node_selection_order *
	map_node_internal(jive_node * node) const;

	/* add ssavars assigned to outputs of node to both vector and set */
	void
	add_node_output_ssavars(
		jive_node * node,
		std::vector<jive_ssavar *> & ssavars,
		std::unordered_set<jive_ssavar *> & unique_ssavars) const;

	/* internal function to push onto node stack */
	void
	push_node_stack_internal(node_selection_order * node_cost);

	/* the master controlling this selector */
	master_selector * master_;
	
	/* the region it operates on */
	const jive_region * region_;
	const jive_shaped_region * shaped_region_;

	/* stack of priorized nodes: these must be selected in LIFO in
	 * preference to any other nodes */
	std::vector<node_selection_order *> node_stack_;
	/* queue of nodes, ordered by priority */
	std::multiset<node_selection_order *, node_selection_order::ptr_less> node_queue_;

	/* hash linkage from master */
	detail::intrusive_hash_anchor<region_selector> hash_chain;
	typedef detail::intrusive_hash_accessor <
		const jive_region *,
		region_selector,
		&region_selector::region_,
		&region_selector::hash_chain
	> hash_chain_accessor;

	friend class master_selector;
};

/* master tying the selectors for indvidual regions together; also provides
 * common support infrastructure */
class master_selector {
public:
	~master_selector();

	master_selector(jive_shaped_graph * shaped_graph);

	inline jive_shaped_graph *
	shaped_graph() const noexcept { return shaped_graph_; }

	/* lookup or create shaper selector for region */
	region_selector *
	map_region(const jive_region * region);

	/* lookup or create cost structure for node */
	node_selection_order *
	map_node(jive_node * node);

	bool
	check_node_selectable(const jive_node * node);

	/* FIXME: all of the below should be private, but they are accessed
	 * in the unit tests */

	/* lookup or create cost structure for node, without validation */
	node_selection_order *
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
	assumed_active(const output * out, const jive_region * region) const noexcept;

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
	node_selection_order *
	node_cost_create(jive_node * node);

	/* create a selector for the given region */
	region_selector *
	region_shaper_selector_create(
		const jive_region * region,
		const jive_shaped_region * shaped_region);

	void
	handle_node_create(jive_node * node);

	void
	handle_input_change(
		input * in, output * old_origin, output * new_origin);

	jive_shaped_graph * shaped_graph_;

	node_selection_order_hash node_map_;

	typedef detail::owner_intrusive_hash <
		const jive_region *,
		region_selector,
		region_selector::hash_chain_accessor
	> region_selector_hash;

	region_selector_hash region_map_;
	computation_tracker cost_computation_state_tracker_;

	std::vector<callback> callbacks_;
};

}
}

#endif
