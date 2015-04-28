/*
 * Copyright 2010 2011 2012 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_REGALLOC_SHAPED_GRAPH_H
#define JIVE_REGALLOC_SHAPED_GRAPH_H

#include <jive/regalloc/assignment-tracker.h>
#include <jive/regalloc/shaped-node.h>
#include <jive/regalloc/shaped-region.h>
#include <jive/regalloc/shaped-variable.h>
#include <jive/util/callbacks.h>

struct jive_graph;

class jive_shaped_graph {
public:
	typedef jive::detail::owner_intrusive_hash<
		const jive_node *,
		jive_shaped_node,
		jive_shaped_node::hash_chain_accessor
	> jive_shaped_node_hash;
	typedef jive::detail::owner_intrusive_hash <
		const jive_variable *,
		jive_shaped_variable,
		jive_shaped_variable::hash_chain_accessor
	> jive_shaped_variable_hash;
	typedef jive::detail::owner_intrusive_hash <
		const jive_ssavar *,
		jive_shaped_ssavar,
		jive_shaped_ssavar::hash_chain_accessor
	> jive_shaped_ssavar_hash;
	typedef jive::detail::owner_intrusive_hash <
		const jive_region *,
		jive_shaped_region,
		jive_shaped_region::hash_chain_accessor
	> jive_shaped_region_hash;


	~jive_shaped_graph();

	jive_shaped_graph(jive_graph * graphx);

	inline jive_shaped_node *
	map_node(const jive_node * node) noexcept
	{
		auto i = node_map_.find(node);
		JIVE_DEBUG_ASSERT(i.ptr());
		return i.ptr();
	}

	inline const jive_shaped_node *
	map_node(const jive_node * node) const noexcept
	{
		auto i = node_map_.find(node);
		JIVE_DEBUG_ASSERT(i.ptr());
		return i.ptr();
	}

	inline jive_shaped_node *
	map_node_location(const jive_node * node) noexcept
	{
		auto i = node_map_.find(node);
		return (i != node_map_.end() && i->cut()) ? i.ptr() : nullptr;
	}

	inline const jive_shaped_node *
	map_node_location(const jive_node * node) const noexcept
	{
		auto i = node_map_.find(node);
		return (i != node_map_.end() && i->cut()) ? i.ptr() : nullptr;
	}

	inline bool
	is_node_placed(const jive_node * node) const noexcept
	{
		const jive_shaped_node * shaped_node = map_node_location(node);
		return shaped_node && shaped_node->cut();
	}

	inline jive_shaped_ssavar *
	map_ssavar(const jive_ssavar * ssavar) noexcept
	{
		auto i = ssavar_map_.find(ssavar);
		return i != ssavar_map_.end() ? i.ptr() : nullptr;
	}

	inline const jive_shaped_ssavar *
	map_ssavar(const jive_ssavar * ssavar) const noexcept
	{
		auto i = ssavar_map_.find(ssavar);
		return i != ssavar_map_.end() ? i.ptr() : nullptr;
	}

	inline jive_shaped_variable *
	map_variable(const jive_variable * variable) noexcept
	{
		auto i = variable_map_.find(variable);
		return i != variable_map_.end() ? i.ptr() : nullptr;
	}

	inline const jive_shaped_variable *
	map_variable(const jive_variable * variable) const noexcept
	{
		auto i = variable_map_.find(variable);
		return i != variable_map_.end() ? i.ptr() : nullptr;
	}

	inline jive_shaped_region *
	map_region(const jive_region * region) noexcept
	{
		auto i = region_map_.find(region);
		return i != region_map_.end() ? i.ptr() : nullptr;
	}

	inline const jive_shaped_region *
	map_region(const jive_region * region) const noexcept
	{
		auto i = region_map_.find(region);
		return i != region_map_.end() ? i.ptr() : nullptr;
	}

	inline jive_graph & graph() const noexcept { return *graph_; }

	std::string
	debug_string() const;

	jive_var_assignment_tracker var_assignment_tracker;

	jive_shaped_node_hash node_map_;
	jive_shaped_ssavar_hash ssavar_map_;
	jive_shaped_variable_hash variable_map_;
	jive_shaped_region_hash region_map_;

	jive::notifier<jive_shaped_node *> on_node_place;
	jive::notifier<jive_shaped_node *> on_node_deplace;
	jive::notifier<jive_shaped_region *, jive_shaped_ssavar *> on_shaped_region_ssavar_add;
	jive::notifier<jive_shaped_region *, jive_shaped_ssavar *> on_shaped_region_ssavar_remove;

private:
	void
	add_region_recursive(jive_region * region);

	jive_graph * graph_;

	std::vector<jive::callback> callbacks_;
};

jive_shaped_graph *
jive_shaped_graph_create(jive_graph * graph);

void
jive_shaped_graph_destroy(jive_shaped_graph * self);

#endif
