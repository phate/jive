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
#include <jive/vsdg/statetype.h>
#include <jive/vsdg/variable.h>

#include <unordered_map>

using namespace std::placeholders;

namespace jive {
namespace regalloc {

static inline jive_resource_class_priority
jive_rescls_priority_min(jive_resource_class_priority a, jive_resource_class_priority b)
{
	return (a < b) ? a : b;
}

/* node_selection_order */

node_selection_order::node_selection_order(master_selector * master, jive_node * node)
	: state_(state_ahead)
	, force_tree_root_(false)
	, node_(node)
	, master_(master)
{
	jive_rescls_prio_array_init(&prio_array_);
	prio_array_.count[0] = jive_resource_class_priority_lowest;
	blocked_rescls_priority_ = jive_resource_class_priority_lowest;
}

node_selection_order::~node_selection_order()
{
}

void
node_selection_order::compute_prio_value()
{
	jive_rescls_prio_array_compute(&prio_array_, &rescls_cost_);
	prio_array_.count[0] = static_cast<size_t>(blocked_rescls_priority_);
}

/* region_selector */

region_selector::region_selector(
	master_selector * master,
	const jive_region * region,
	const jive_shaped_region * shaped_region)
	: master_(master)
	, region_(region)
	, shaped_region_(shaped_region)
{
}

void
region_selector::push_node_stack_internal(node_selection_order * node_cost)
{
	JIVE_DEBUG_ASSERT(region_ == node_cost->node()->region);
	switch (node_cost->state_) {
		case node_selection_order::state_ahead:
			break;
		case node_selection_order::state_queue:
			node_queue_.erase(node_cost->queue_index_);
			break;
		case node_selection_order::state_stack:
			return;
		case node_selection_order::state_done:
			return;
	}
	
	node_cost->state_ = node_selection_order::state_stack;
	node_cost->stack_index_ = node_stack_.size();
	node_stack_.push_back(node_cost);
}

void
region_selector::push_node_stack(
	jive_node * node)
{
	node_selection_order * node_cost = map_node_internal(node);
	push_node_stack_internal(node_cost);
}

jive_node *
region_selector::select_node()
{
	if (!node_stack_.empty()) {
		return node_stack_.back()->node();
	}
	
	if (!node_queue_.empty()) {
		master_->revalidate();
		node_selection_order * node_cost = *node_queue_.begin();
		push_node_stack_internal(node_cost);
		return node_cost->node();
	}
	
	return nullptr;
}

void
region_selector::add_node_output_ssavars(
	jive_node * node,
	std::vector<jive_ssavar *> & ssavars,
	std::unordered_set<jive_ssavar *> & unique_ssavars) const
{
	for (size_t n = 0; n < node->noutputs; n++) {
		jive_shaped_ssavar * shaped_ssavar = jive_varcut_map_output(
			&shaped_region_->active_top.base, node->outputs[n]);
		if (shaped_ssavar) {
			ssavars.push_back(shaped_ssavar->ssavar);
			unique_ssavars.insert(shaped_ssavar->ssavar);
		}
	}
}

node_selection_order *
region_selector::map_node_internal(jive_node * node) const
{
	return master_->map_node_internal(node);
}

std::vector<jive_ssavar *>
region_selector::prio_sorted_ssavars() const
{
	/* ssavars sorted by priority, highest priority ssavars first */
	std::vector<jive_ssavar *> sorted_ssavars;
	
	/* ssavars we have seen already, so we don't add them twice */
	std::unordered_set<jive_ssavar *> seen_ssavars;

	/* FIXME: mix in imported origins once they are tracked as well */
	
	/* add ssavars produced by nodes to be scheduled soon in priority order
	 * to the beginning of the list -- we want to avoid spilling them */
	
	/* highest priority (least preference to spill): nodes in the priority
	 * stack */
	for (auto i = node_stack_.rbegin(); i != node_stack_.rend(); ++i) {
		add_node_output_ssavars((*i)->node(), sorted_ssavars, seen_ssavars);
	}

	/* second highest: schedulable nodes order by scheduling cost */
	for (node_selection_order * node_cost : node_queue_) {
		add_node_output_ssavars(node_cost->node(), sorted_ssavars, seen_ssavars);
	}
	
	/* lowest: all remaining ssavars to be considered; we don't care about
	 * relative order */
	jive_cutvar_xpoint * xpoint;
	JIVE_LIST_ITERATE(shaped_region_->active_top.base.xpoints, xpoint, varcut_xpoints_list) {
		jive_ssavar * ssavar = xpoint->shaped_ssavar->ssavar;
		if (seen_ssavars.find(ssavar) == seen_ssavars.end()) {
			sorted_ssavars.push_back(ssavar);
		}
	}
	
	return sorted_ssavars;
}

jive_ssavar *
region_selector::select_spill(
	const jive_resource_class * rescls,
	jive_node * disallow_origins) const
{
	std::vector<jive_ssavar *> prio_ssavars = prio_sorted_ssavars();
	for (auto i = prio_ssavars.rbegin(); i != prio_ssavars.rend(); ++i) {
		jive_ssavar * ssavar = *i;
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
	return nullptr;
}

/* master_selector */

master_selector::~master_selector()
{
}

master_selector::master_selector(jive_shaped_graph * shaped_graph)
	: shaped_graph_(shaped_graph)
	, cost_computation_state_tracker_(shaped_graph->graph)
{
	jive_graph * graph = shaped_graph->graph;
	
	init_region_recursive(graph->root_region);
	jive_node * node;
	JIVE_LIST_ITERATE(graph->bottom, node, graph_bottom_list) {
		try_add_frontier(node);
	}
	
	callbacks_.push_back(graph->on_node_create.connect(
		std::bind(&master_selector::handle_node_create, this, _1)));
	callbacks_.push_back(graph->on_input_change.connect(
		std::bind(&master_selector::handle_input_change, this, _1, _2, _3)));
	callbacks_.push_back(shaped_graph->on_shaped_node_create.connect(
		[this](jive_node * node){
			this->mark_shaped(node);
		}));
	callbacks_.push_back(shaped_graph->on_shaped_region_ssavar_add.connect(
		[this](jive_shaped_region * shaped_region, jive_shaped_ssavar * shaped_ssavar) {
			this->invalidate_node(shaped_ssavar->ssavar->origin->node());
		}));
	callbacks_.push_back(shaped_graph->on_shaped_region_ssavar_remove.connect(
		[this](jive_shaped_region * shaped_region, jive_shaped_ssavar * shaped_ssavar) {
			this->invalidate_node(shaped_ssavar->ssavar->origin->node());
		}));
}

region_selector *
master_selector::map_region(const jive_region * region)
{
	region_selector * region_selector = nullptr;
	auto i = region_map_.find(region);
	if (i != region_map_.end())
		region_selector = i.ptr();

	if (!region_selector) {
		jive_shaped_region * shaped_region = jive_shaped_graph_map_region(shaped_graph_, region);
		region_selector = region_shaper_selector_create(region, shaped_region);
	}
	return region_selector;
}

node_selection_order *
master_selector::map_node(jive_node * node)
{
	revalidate();
	return map_node_internal(node);
}

bool
master_selector::check_node_selectable(const jive_node * node)
{
	for (size_t n = 0; n < node->noutputs; n++) {
		output * out = node->outputs[n];
		input * user;
		JIVE_LIST_ITERATE(out->users, user, output_users_list) {
			node_selection_order * node_cost = map_node_internal(user->node());
			if (node_cost->state_ != node_selection_order::state_done)
				return false;
		}
	}
	return true;
}

node_selection_order *
master_selector::map_node_internal(jive_node * node)
{
	auto i = node_map_.find(node);
	if (i != node_map_.end()) {
		return i.ptr();
	} else {
		return node_cost_create(node);
	}
}

void
master_selector::invalidate_node(jive_node * node)
{
	cost_computation_state_tracker_.invalidate(node);
}

void
master_selector::revalidate_node(jive_node * node)
{
	node_selection_order * node_cost;
	node_cost = map_node_internal(node);
	
	jive_resource_class_count cost;
	compute_node_cost(&cost, node);
	bool force_tree_root = !maybe_inner_node(node);
	jive_resource_class_priority blocked_rescls_priority;
	blocked_rescls_priority = compute_blocked_rescls_priority(node);
	
	bool changes =
		(force_tree_root != node_cost->force_tree_root_) ||
		(blocked_rescls_priority != node_cost->blocked_rescls_priority_) ||
		(cost != node_cost->rescls_cost_);
	
	if (changes) {
		region_selector * region_shaper;
		region_shaper = map_region(node->region);
		
		if (node_cost->state_ == node_selection_order::state_queue) {
			region_shaper->node_queue_.erase(node_cost->queue_index_);
		}
		
		node_cost->force_tree_root_ = force_tree_root;
		node_cost->blocked_rescls_priority_ = blocked_rescls_priority;
		node_cost->rescls_cost_ = cost;
		node_cost->compute_prio_value();
		
		if (node_cost->state_ == node_selection_order::state_queue){
			node_cost->queue_index_ = region_shaper->node_queue_.insert(node_cost);
		}
		
		cost_computation_state_tracker_.invalidate_below(node);
	}
	
}

void
master_selector::revalidate()
{
	jive_node * node;
	node = cost_computation_state_tracker_.pop_top();
	while (node) {
		revalidate_node(node);
		node = cost_computation_state_tracker_.pop_top();
	}
}

bool
master_selector::assumed_active(
	const output * out, const jive_region * region) const noexcept
{
	jive_shaped_region * shaped_region = jive_shaped_graph_map_region(shaped_graph_, region);
	return jive_region_varcut_output_is_active(&shaped_region->active_top, out);
}

bool
master_selector::maybe_inner_node(const jive_node * node) const noexcept
{
	if (jive_shaped_graph_map_node(shaped_graph_, node))
		return false;
	
	size_t user_count = 0;
	size_t nonstate_count = 0;
	bool other_region = false;
	for (size_t n = 0; n < node->noutputs; n++) {
		output * out = node->outputs[n];
		
		size_t output_user_count = 0;
		bool other_region = false;
		input * user;
		JIVE_LIST_ITERATE(out->users, user, output_users_list) {
			output_user_count ++;
			other_region = other_region || (user->node()->region != node->region);
		}
		
		if (!output_user_count)
			continue;
		
		if (!dynamic_cast<const state::type*>(&out->type()))
			nonstate_count ++;
		
		user_count += output_user_count;
	}
	
	return (user_count <= 1) && (nonstate_count >= 1) && (!other_region);
}

void
master_selector::compute_node_cost(
	jive_resource_class_count * cost,
	jive_node * node)
{
	if (jive_shaped_graph_map_node(shaped_graph_, node)) {
		cost->clear();
		return;
	}
	
	jive_region * region = node->region;
	
	jive_resource_class_count output_cost, input_cost, self_cost, eval_cost, this_eval_cost;
	
	/* compute cost of self-representation */
	for (size_t n = 0; n < node->noutputs; n++) {
		output_cost.add(jive_resource_class_relax(node->outputs[n]->required_rescls));
	}
	
	/* consider cost of all inputs */
	for (size_t n = 0; n < node->ninputs; n++) {
		input * in = node->inputs[n];
		if (assumed_active(in->origin(), region)) {
			continue;
		}
		input_cost.add(jive_resource_class_relax(in->required_rescls));
	}
	
	self_cost = output_cost;
	self_cost.update_union(input_cost);
	
	bool first = true;
	for (size_t n = 0; n < node->ninputs; n++) {
		input * in = node->inputs[n];
		node_selection_order * last_eval = map_node(in->producer());
		
		if (last_eval->force_tree_root_) {
			continue;
		}
		this_eval_cost = input_cost;
		if (!assumed_active(in->origin(), region)) {
			const jive_resource_class * rescls = jive_resource_class_relax(in->required_rescls);
			this_eval_cost.add(rescls);
		}
		this_eval_cost.update_add(last_eval->rescls_cost_);
		
		if (first) {
			eval_cost = this_eval_cost;
		} else {
			eval_cost.update_intersection(this_eval_cost);
		}
		
		first = false;
	}
	
	if (!first) {
		self_cost.update_union(eval_cost);
	}
	
	/* remove cost for those that would be removed by picking this node */
	for (size_t n = 0; n < node->noutputs; n++) {
		if (assumed_active(node->outputs[n], region)) {
			const jive_resource_class * rescls = jive_resource_class_relax(
				node->outputs[n]->required_rescls);
			self_cost.sub(rescls);
		}
	}

	*cost = self_cost;
}

jive_resource_class_priority
master_selector::compute_blocked_rescls_priority(jive_node * node)
{
	if (jive_shaped_graph_map_node(shaped_graph_, node))
		return jive_root_resource_class.priority;
	
	jive_region * region = node->region;
	jive_resource_class_priority blocked_rescls_priority = jive_root_resource_class.priority;
	
	for (size_t n = 0; n < node->noutputs; n++) {
		output * out = node->outputs[n];
		if (assumed_active(out, region)) {
			const jive_resource_class * rescls = jive_resource_class_relax(out->required_rescls);
			blocked_rescls_priority = jive_rescls_priority_min(blocked_rescls_priority, rescls->priority);
		}
	}
	
	for (size_t n = 0; n < node->ninputs; n++) {
		input * in = node->inputs[n];
		node_selection_order * upper = map_node(in->producer());
		
		const jive_resource_class * input_rescls = jive_resource_class_relax(in->required_rescls);
		
		if (upper->force_tree_root_) {
			if (assumed_active(in->origin(), region)) {
				blocked_rescls_priority = jive_rescls_priority_min(blocked_rescls_priority,
					input_rescls->priority);
			}
		} else {
			blocked_rescls_priority = jive_rescls_priority_min(blocked_rescls_priority,
				upper->blocked_rescls_priority_);
		}
	}
	
	return blocked_rescls_priority;
}

void
master_selector::try_add_frontier(jive_node * node)
{
	if (!check_node_selectable(node)) {
		return;
	}
	node_selection_order * node_cost = map_node_internal(node);
	if (node_cost->state_ != node_selection_order::state_ahead) {
		return;
	}
	
	node_cost->state_ = node_selection_order::state_queue;
	region_selector * region_shaper;
	region_shaper = map_region(node->region);
	node_cost->queue_index_ = region_shaper->node_queue_.insert(node_cost);
}

bool
master_selector::remove_frontier(jive_node * node)
{
	node_selection_order * node_cost = map_node_internal(node);
	
	if (node_cost->state_ == node_selection_order::state_stack) {
		return true;
	}
	
	if (node_cost->state_ != node_selection_order::state_queue) {
		return false;
	}
	
	region_selector * region_shaper;
	region_shaper = map_region(node->region);
	region_shaper->node_queue_.erase(node_cost->queue_index_);
	
	node_cost->state_ = node_selection_order::state_ahead;
	
	return false;
}

void
master_selector::mark_shaped(jive_node * node)
{
	node_selection_order * node_cost = map_node_internal(node);
	
	region_selector * region_shaper = map_region(node->region);
	switch (node_cost->state_) {
		case node_selection_order::state_queue: {
			region_shaper->node_queue_.erase(node_cost->queue_index_);
			break;
		}
		case node_selection_order::state_stack: {
			size_t index = node_cost->stack_index_;
			region_shaper->node_stack_.erase(region_shaper->node_stack_.begin() + index);
			while (index < region_shaper->node_stack_.size()) {
				region_shaper->node_stack_[index]->stack_index_ = index;
				++ index;
			}
			break;
		}
		case node_selection_order::state_ahead: {
			abort();
			break;
		}
		case node_selection_order::state_done: {
			abort();
			break;
		}
	}
	
	invalidate_node(node);
	node_cost->state_ = node_selection_order::state_done;
	
	for (size_t n = 0; n < node->ninputs; n++) {
		try_add_frontier(node->inputs[n]->producer());
	}
}

void
master_selector::init_region_recursive(jive_region * region)
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

node_selection_order *
master_selector::node_cost_create(jive_node * node)
{
	std::unique_ptr<node_selection_order> cost(new node_selection_order(this, node));
	return node_map_.insert(std::move(cost)).ptr();
}

region_selector *
master_selector::region_shaper_selector_create(
	const jive_region * region,
	const jive_shaped_region * shaped_region)
{
	std::unique_ptr<region_selector> self(
		new region_selector(this, region, shaped_region));

	return region_map_.insert(std::move(self)).ptr();
}

void
master_selector::handle_node_create(jive_node * node)
{
	bool stacked = false;
	for (size_t n = 0; n < node->ninputs; n++) {
		input * in = node->inputs[n];
		invalidate_node(in->producer());
		
		if (remove_frontier(in->producer())) {
			stacked = true;
		}
	}
	
	invalidate_node(node);
	try_add_frontier(node);
	
	/* if any of the inputs was on the priority stack, then this needs
	 * to go there as well */
	if (stacked) {
		map_region(node->region)->push_node_stack_internal(
			map_node_internal(node));
	}
}

void
master_selector::handle_input_change(
	input * in, output * old_origin, output * new_origin)
{
	try_add_frontier(old_origin->node());
	
	node_selection_order * upper_node_cost = map_node_internal(new_origin->node());
	node_selection_order * lower_node_cost = map_node_internal(in->node());
	
	if (lower_node_cost->state_ == node_selection_order::state_ahead) {
		if (upper_node_cost->state_ == node_selection_order::state_queue) {
			remove_frontier(new_origin->node());
		}
	}
	
	if (upper_node_cost->state_ == node_selection_order::state_stack) {
		if (lower_node_cost->state_ != node_selection_order::state_done) {
			map_region(in->node()->region)->push_node_stack_internal(
				map_node_internal(in->node()));
		}
	}
}

}
}
