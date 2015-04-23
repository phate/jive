/*
 * Copyright 2015 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/regalloc/selector-simple.h>

#include <jive/common.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/shaped-variable.h>
#include <jive/regalloc/xpoint.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/statetype.h>
#include <jive/vsdg/traverser.h>
#include <jive/vsdg/variable.h>

#include <unordered_map>

namespace jive {
namespace regalloc {

/* region_selector_simple */

region_selector_simple::~region_selector_simple() noexcept {}

region_selector_simple::region_selector_simple(
	master_selector_simple * master,
	const jive_region * region,
	const jive_shaped_region * shaped_region)
	: master_(master)
	, region_(region)
	, shaped_region_(shaped_region)
{
}

void
region_selector_simple::push_node_stack(
	jive_node * node)
{
	for (auto i = node_sequence_.begin(); i != node_sequence_.end(); ++i) {
		if (*i == node) {
			node_sequence_.erase(i);
			break;
		}
	}
	node_sequence_.push_back(node);
}

jive_node *
region_selector_simple::select_node()
{
	if (!node_sequence_.empty()) {
		return node_sequence_.back();
	} else {
		return nullptr;
	}
}

void
region_selector_simple::add_node_output_ssavars(
	jive_node * node,
	std::vector<jive_ssavar *> & ssavars,
	std::unordered_set<jive_ssavar *> & unique_ssavars) const
{
	for (size_t n = 0; n < node->noutputs; n++) {
		jive_shaped_ssavar * shaped_ssavar =
			shaped_region_->active_top().map_output(node->outputs[n]);
		if (shaped_ssavar) {
			ssavars.push_back(&shaped_ssavar->ssavar());
			unique_ssavars.insert(&shaped_ssavar->ssavar());
		}
	}
}

std::vector<jive_ssavar *>
region_selector_simple::prio_sorted_ssavars() const
{
	/* ssavars sorted by priority, highest priority ssavars first */
	std::vector<jive_ssavar *> sorted_ssavars;
	
	/* ssavars we have seen already, so we don't add them twice */
	std::unordered_set<jive_ssavar *> seen_ssavars;

	/* FIXME: mix in imported origins once they are tracked as well */
	
	/* add ssavars produced by nodes to be scheduled soon in priority order
	 * to the beginning of the list -- we want to avoid spilling them */

	for (auto i = node_sequence_.rbegin(); i != node_sequence_.rend(); ++i) {
		add_node_output_ssavars(*i, sorted_ssavars, seen_ssavars);
	}

	/* lowest: all remaining ssavars to be considered; we don't care about
	 * relative order */
	for (const jive_cutvar_xpoint & xpoint : shaped_region_->active_top()) {
		jive_ssavar * ssavar = &xpoint.shaped_ssavar()->ssavar();
		if (seen_ssavars.find(ssavar) == seen_ssavars.end()) {
			sorted_ssavars.push_back(ssavar);
		}
	}
	
	return sorted_ssavars;
}

jive_ssavar *
region_selector_simple::select_spill(
	const jive_resource_class * rescls,
	jive_node * disallow_origins) const
{
	std::vector<jive_ssavar *> prio_ssavars = prio_sorted_ssavars();
	for (auto i = prio_ssavars.rbegin(); i != prio_ssavars.rend(); ++i) {
		jive_ssavar * ssavar = *i;
		if (!jive_variable_may_spill(ssavar->variable)) {
			continue;
		}
		
		if (ssavar->origin->node() == disallow_origins) {
			continue;
		}
		
		const jive_resource_class * var_rescls = jive_variable_get_resource_class(ssavar->variable);
		if (jive_resource_class_intersection(rescls, var_rescls) != var_rescls) {
			continue;
		}
		
		return ssavar;
	}
	
	JIVE_DEBUG_ASSERT(false);
	return nullptr;
}

/* master_selector */

master_selector_simple::~master_selector_simple() noexcept
{
}

master_selector_simple::master_selector_simple(jive_shaped_graph * shaped_graph)
	: shaped_graph_(shaped_graph)
{
	callbacks_.push_back(shaped_graph->graph().on_node_create.connect(
		std::bind(&master_selector_simple::handle_node_create, this, std::placeholders::_1)));
	callbacks_.push_back(shaped_graph->on_node_place.connect(
		[this](jive_shaped_node * shaped_node){
			this->mark_shaped(shaped_node->node());
		}));

	for (jive_node * node : jive::bottomup_traverser(&shaped_graph_->graph())) {
		map_region(node->region)->node_sequence_.push_front(node);
	}
}

region_selector_simple *
master_selector_simple::map_region(const jive_region * region)
{
	auto i = region_map_.find(region);
	return
		i != region_map_.end() ?
		i.ptr() :
		region_shaper_selector_simple_create(region, shaped_graph_->map_region(region));
}


void
master_selector_simple::mark_shaped(jive_node * node)
{
	region_selector_simple * r = map_region(node->region);
	auto i = r->node_sequence_.begin();
	while (i != r->node_sequence_.end() && *i != node) {
		++i;
	}
	JIVE_DEBUG_ASSERT(i != r->node_sequence_.end());
	if (i != r->node_sequence_.end()) {
		r->node_sequence_.erase(i);
	}
}

region_selector_simple *
master_selector_simple::region_shaper_selector_simple_create(
	const jive_region * region,
	const jive_shaped_region * shaped_region)
{
	std::unique_ptr<region_selector_simple> self(
		new region_selector_simple(this, region, shaped_region));

	return region_map_.insert(std::move(self)).ptr();
}

void
master_selector_simple::handle_node_create(jive_node * node)
{
	region_selector_simple * r = map_region(node->region);
	auto pos = r->node_sequence_.begin();
	for (auto i = r->node_sequence_.begin(); i != r->node_sequence_.end(); ++i) {
		bool is_pred = false;
		for (size_t n = 0; n < node->ninputs; ++n) {
			is_pred = is_pred || node->inputs[n]->origin()->node() == *i;
		}
		if (is_pred) {
			pos = std::next(i);
		}
	}

	r->node_sequence_.insert(pos, node);
}

}
}
