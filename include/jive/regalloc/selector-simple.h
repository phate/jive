/*
 * Copyright 2015 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_REGALLOC_SELECTOR_SIMPLE_H
#define JIVE_REGALLOC_SELECTOR_SIMPLE_H

#include <stdbool.h>
#include <stddef.h>

#include <list>
#include <set>
#include <unordered_set>
#include <vector>

#include <jive/regalloc/selector.h>
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

class master_selector_simple;

/* select next nodes to be shaped in region */
class region_selector_simple final : public region_selector {
public:
	virtual
	~region_selector_simple() noexcept;

	region_selector_simple(
		master_selector_simple * master,
		const jive_region * region,
		const jive_shaped_region * shaped_region);

	/* select next node for shaping */
	virtual
	jive_node *
	select_node() override;

	/* select variable suitable for spilling */
	virtual
	jive_ssavar *
	select_spill(
		const jive_resource_class * rescls,
		jive_node * disallow_origins) const override;

	/* push onto priority node selection stack -- this node will be
	 * preferred for selection over all other currently selectable nodes */
	virtual
	void
	push_node_stack(
		jive_node * node) override;

private:
	/* ssavars sorted by priority, highest priority ssavars first */
	std::vector<jive_ssavar *>
	prio_sorted_ssavars() const;

	/* add ssavars assigned to outputs of node to both vector and set */
	void
	add_node_output_ssavars(
		jive_node * node,
		std::vector<jive_ssavar *> & ssavars,
		std::unordered_set<jive_ssavar *> & unique_ssavars) const;

	/* the master controlling this selector */
	master_selector_simple * master_;
	
	/* the region it operates on */
	const jive_region * region_;
	const jive_shaped_region * shaped_region_;


	/* hash linkage from master */
	detail::intrusive_hash_anchor<region_selector_simple> hash_chain_;
	typedef detail::intrusive_hash_accessor <
		const jive_region *,
		region_selector_simple,
		&region_selector_simple::region_,
		&region_selector_simple::hash_chain_
	> hash_chain_accessor;

	std::list<jive_node *> node_sequence_;

	friend class master_selector_simple;
};

/* master tying the selectors for indvidual regions together; also provides
 * common support infrastructure */
class master_selector_simple final : public master_selector {
public:
	virtual
	~master_selector_simple() noexcept;

	master_selector_simple(jive_shaped_graph * shaped_graph);

	inline jive_shaped_graph *
	shaped_graph() const noexcept { return shaped_graph_; }

	/* lookup or create shaper selector for region */
	virtual
	region_selector_simple *
	map_region(const jive_region * region) override;

private:
	void
	sequentialize_regions();

	void
	handle_node_create(
		jive_node * node);

	void
	handle_input_change(
		input * in, output * old_origin, output * new_origin);

	void
	mark_shaped(
		jive_node * node);

	region_selector_simple *
	region_shaper_selector_simple_create(
		const jive_region * region,
		const jive_shaped_region * shaped_region);

	jive_shaped_graph * shaped_graph_;

	typedef detail::owner_intrusive_hash <
		const jive_region *,
		region_selector_simple,
		region_selector_simple::hash_chain_accessor
	> region_selector_simple_hash;

	region_selector_simple_hash region_map_;

	std::vector<callback> callbacks_;
};

}
}

#endif
