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

typedef struct jive_shaped_graph jive_shaped_graph;

struct jive_graph;

struct jive_shaped_graph {
public:
	~jive_shaped_graph();

	struct jive_graph * graph;
	
	jive_var_assignment_tracker var_assignment_tracker;

	jive_shaped_node_hash node_map;
	jive_shaped_ssavar_hash ssavar_map;
	jive_shaped_variable_hash variable_map;
	jive_shaped_region_hash region_map;

	jive::notifier<jive_node *> on_shaped_node_create;
	jive::notifier<jive_node *> on_shaped_node_destroy;
	jive::notifier<jive_shaped_region *, jive_shaped_ssavar *> on_shaped_region_ssavar_add;
	jive::notifier<jive_shaped_region *, jive_shaped_ssavar *> on_shaped_region_ssavar_remove;

	std::vector<jive::callback> callbacks_;
};

jive_shaped_graph *
jive_shaped_graph_create(struct jive_graph * graph);

void
jive_shaped_graph_destroy(jive_shaped_graph * self);

inline jive_shaped_region *
jive_shaped_graph_map_region(
	jive_shaped_graph * self, const jive_region * region) noexcept
{
	auto i = self->region_map.find(region);
	return i != self->region_map.end() ? i.ptr() : nullptr;
}

inline const jive_shaped_region *
jive_shaped_graph_map_region(
	const jive_shaped_graph * self, const jive_region * region) noexcept
{
	auto i = self->region_map.find(region);
	return i != self->region_map.end() ? i.ptr() : nullptr;
}

inline jive_shaped_variable *
jive_shaped_graph_map_variable(
	jive_shaped_graph * self, const jive_variable * variable) noexcept
{
	auto i = self->variable_map.find(variable);
	return i != self->variable_map.end() ? i.ptr() : nullptr;
}

inline const jive_shaped_variable *
jive_shaped_graph_map_variable(
	const jive_shaped_graph * self, const jive_variable * variable) noexcept
{
	auto i = self->variable_map.find(variable);
	return i != self->variable_map.end() ? i.ptr() : nullptr;
}

inline jive_shaped_ssavar *
jive_shaped_graph_map_ssavar(
	jive_shaped_graph * self, const jive_ssavar * ssavar) noexcept
{
	auto i = self->ssavar_map.find(ssavar);
	return i != self->ssavar_map.end() ? i.ptr() : nullptr;
}

inline const jive_shaped_ssavar *
jive_shaped_graph_map_ssavar(
	const jive_shaped_graph * self, const jive_ssavar * ssavar) noexcept
{
	auto i = self->ssavar_map.find(ssavar);
	return i != self->ssavar_map.end() ? i.ptr() : nullptr;
}

inline jive_shaped_node *
jive_shaped_graph_map_node(
	jive_shaped_graph * self, const jive_node * node) noexcept
{
	auto i = self->node_map.find(node);
	return i != self->node_map.end() ? i.ptr() : nullptr;
}

inline const jive_shaped_node *
jive_shaped_graph_map_node(
	const jive_shaped_graph * self, const jive_node * node) noexcept
{
	auto i = self->node_map.find(node);
	return i != self->node_map.end() ? i.ptr() : nullptr;
}

#endif
