/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_REGALLOC_SHAPED_GRAPH_H
#define JIVE_REGALLOC_SHAPED_GRAPH_H

#include <jive/regalloc/assignment-tracker.h>
#include <jive/regalloc/notifiers.h>
#include <jive/regalloc/shaped-node.h>
#include <jive/regalloc/shaped-region.h>
#include <jive/regalloc/shaped-variable.h>

typedef struct jive_shaped_graph jive_shaped_graph;

struct jive_graph;

struct jive_shaped_graph {
	~jive_shaped_graph();

	struct jive_graph * graph;
	
	struct jive_notifier * callbacks[21];

	jive_var_assignment_tracker var_assignment_tracker;

	jive_shaped_node_hash node_map;
	jive_shaped_ssavar_hash ssavar_map;
	jive_shaped_variable_hash variable_map;
	jive_shaped_region_hash region_map;

	jive_node_notifier_slot on_shaped_node_create;
	jive_node_notifier_slot on_shaped_node_destroy;
	jive_shaped_region_ssavar_notifier_slot on_shaped_region_ssavar_add;
	jive_shaped_region_ssavar_notifier_slot on_shaped_region_ssavar_remove;
};

jive_shaped_graph *
jive_shaped_graph_create(struct jive_graph * graph);

void
jive_shaped_graph_destroy(jive_shaped_graph * self);

inline jive_shaped_region *
jive_shaped_graph_map_region(jive_shaped_graph * self, const jive_region * region)
{
	auto i = self->region_map.find(region);
	return i != self->region_map.end() ? i.ptr() : nullptr;
}

inline const jive_shaped_region *
jive_shaped_graph_map_region(const jive_shaped_graph * self, const jive_region * region)
{
	auto i = self->region_map.find(region);
	return i != self->region_map.end() ? i.ptr() : nullptr;
}

inline jive_shaped_variable *
jive_shaped_graph_map_variable(jive_shaped_graph * self, const jive_variable * variable)
{
	auto i = self->variable_map.find(variable);
	return i != self->variable_map.end() ? i.ptr() : nullptr;
}

inline const jive_shaped_variable *
jive_shaped_graph_map_variable(const jive_shaped_graph * self, const jive_variable * variable)
{
	auto i = self->variable_map.find(variable);
	return i != self->variable_map.end() ? i.ptr() : nullptr;
}

inline jive_shaped_ssavar *
jive_shaped_graph_map_ssavar(jive_shaped_graph * self, const jive_ssavar * ssavar)
{
	auto i = self->ssavar_map.find(ssavar);
	return i != self->ssavar_map.end() ? i.ptr() : nullptr;
}

inline const jive_shaped_ssavar *
jive_shaped_graph_map_ssavar(const jive_shaped_graph * self, const jive_ssavar * ssavar)
{
	auto i = self->ssavar_map.find(ssavar);
	return i != self->ssavar_map.end() ? i.ptr() : nullptr;
}

inline jive_shaped_node *
jive_shaped_graph_map_node(jive_shaped_graph * self, const jive_node * node)
{
	auto i = self->node_map.find(node);
	return i != self->node_map.end() ? i.ptr() : nullptr;
}

inline const jive_shaped_node *
jive_shaped_graph_map_node(const jive_shaped_graph * self, const jive_node * node)
{
	auto i = self->node_map.find(node);
	return i != self->node_map.end() ? i.ptr() : nullptr;
}

#endif
