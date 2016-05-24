/*
 * Copyright 2010 2011 2012 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/regalloc/shaped-variable.h>

#include <jive/common.h>
#include <jive/regalloc/crossing-arc.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/shaped-region.h>
#include <jive/regalloc/xpoint.h>
#include <jive/vsdg/gate-interference-private.h>

jive_shaped_variable::interference_item *
jive_variable_interference_create(jive_shaped_variable * first, jive_shaped_variable * second)
{
	/* a variable may not interfere with itself */
	JIVE_DEBUG_ASSERT(first != second);
	
	jive_shaped_variable::interference_item * i = new jive_shaped_variable::interference_item;
	i->first.shaped_variable = first;
	i->first.whole = i;
	i->second.shaped_variable = second;
	i->second.whole = i;
	i->count = 0;
	
	first->interference.insert(&i->second);
	second->interference.insert(&i->first);
	
	return i;
}

static void
jive_variable_interference_destroy(jive_shaped_variable::interference_item * self)
{
	self->first.shaped_variable->interference.erase(&self->second);
	self->second.shaped_variable->interference.erase(&self->first);
	delete self;
}

/* jive_var_assignment_tracker implementation bits */

void
jive_var_assignment_tracker::add_tracked(
	jive_shaped_variable * shaped_variable,
	const jive_resource_class * rescls,
	const jive_resource_name * resname)
{
	if (resname || rescls->limit == 0) {
		JIVE_LIST_PUSH_BACK(assigned, shaped_variable, assignment_variable_list_);
	} else if (shaped_variable->allowed_names_.size() > shaped_variable->squeeze()) {
		JIVE_LIST_PUSH_BACK(trivial, shaped_variable, assignment_variable_list_);
	} else {
		size_t index = shaped_variable->squeeze() - shaped_variable->allowed_names_.size();
		if (index >= pressured.size()) {
			pressured.resize(index + 1, {nullptr, nullptr});
		}
		JIVE_LIST_PUSH_BACK(pressured[index], shaped_variable, assignment_variable_list_);
	}
}

void
jive_var_assignment_tracker::remove_tracked(
	jive_shaped_variable * shaped_variable,
	const jive_resource_class * rescls,
	const jive_resource_name * resname)
{
	if (resname || rescls->limit == 0) {
		JIVE_LIST_REMOVE(assigned, shaped_variable, assignment_variable_list_);
	} else if (shaped_variable->allowed_names_.size() > shaped_variable->squeeze()) {
		JIVE_LIST_REMOVE(trivial, shaped_variable, assignment_variable_list_);
	} else {
		size_t index = shaped_variable->squeeze() - shaped_variable->allowed_names_.size();
		JIVE_LIST_REMOVE(pressured[index], shaped_variable, assignment_variable_list_);
		while (!pressured.empty() && !pressured.rbegin()->first) {
			pressured.resize(pressured.size() - 1);
		}
	}
}

/* jive_shaped_variable implementation bits */

jive_shaped_variable::~jive_shaped_variable()
{
	jive::gate * gate;
	JIVE_LIST_ITERATE(variable_->gates, gate, variable_gate_list) {
		for (auto i = gate->interference.begin(); i != gate->interference.end(); i++) {
			jive::gate * other_gate = i->gate;
			jive_variable * other = other_gate->variable;
			if (!other) continue;
			jive_shaped_variable * other_shape = shaped_graph().map_variable(other);
			if (other_shape) {
				jive_shaped_variable::interference_remove(this, other_shape);
			}
		}
	}
	shaped_graph().var_assignment_tracker.remove_tracked(
		this, variable_->rescls, variable_->resname);

	auto i = interference.begin();
	while (i != interference.end()) {
		interference_item * item = i->whole;
		++i;
		jive_variable_interference_destroy(item);
	}
	JIVE_DEBUG_ASSERT(interference.empty());
}

jive_shaped_variable::jive_shaped_variable(
	jive_shaped_graph * shaped_graph,
	jive_variable * variable)
	: shaped_graph_(shaped_graph)
	, variable_(variable)
{
	internal_recompute_allowed_names();
	shaped_graph_->var_assignment_tracker.add_tracked(
		this, variable_->rescls, variable_->resname);
}

void
jive_shaped_variable::internal_recompute_allowed_names()
{
	squeeze_ = 0;
	allowed_names_.clear();
	
	if (variable()->resname) {
		allowed_names_.insert(variable()->resname);
	} else if (variable()->rescls->limit) {
		size_t nnames;
		const jive_resource_name * const * names;
		jive_resource_class_get_resource_names(variable()->rescls, &nnames, &names);
		for(size_t n = 0; n < nnames; n++) {
			allowed_names_.insert(names[n]);
		}
		
		for (const auto & part : interference) {
			jive_shaped_variable * other = part.shaped_variable;
			if (other->variable()->resname) {
				allowed_names_.erase(other->variable()->resname);
			} else if (other->variable()->rescls->limit) {
				const jive_resource_class * rescls;
				rescls = jive_resource_class_intersection(variable()->rescls, other->variable()->rescls);
				if (rescls) {
					squeeze_ ++;
				}
			}
		}
	}
}


jive_shaped_variable *
jive_shaped_variable_create(
	jive_shaped_graph * shaped_graph,
	jive_variable * variable)
{
	return shaped_graph->variable_map_.insert(std::unique_ptr<jive_shaped_variable>(
		new jive_shaped_variable(shaped_graph, variable))).ptr();
}

void
jive_shaped_variable::resource_class_change(
	const jive_resource_class * old_rescls,
	const jive_resource_class * new_rescls)
{
	jive_ssavar * ssavar;
	JIVE_LIST_ITERATE(variable_->ssavars, ssavar, variable_ssavar_list) {
		shaped_graph().map_ssavar(ssavar)->xpoints_change_resource_class(old_rescls, new_rescls);
	}
	
	if (!variable_->resname) {
		for (const auto & part : interference) {
			jive_shaped_variable * other = part.shaped_variable;
			other->sub_squeeze(old_rescls);
			other->add_squeeze(new_rescls);
		}
	}
	shaped_graph_->var_assignment_tracker.remove_tracked(
		this, old_rescls, variable_->resname);
	internal_recompute_allowed_names();
	shaped_graph_->var_assignment_tracker.add_tracked(
		this, new_rescls, variable_->resname);
}

void
jive_shaped_variable::resource_name_change(
	const jive_resource_name * old_resname,
	const jive_resource_name * new_resname)
{
	JIVE_DEBUG_ASSERT(
		old_resname == new_resname ||
		variable_->rescls->limit == 0 ||
		allowed_resource_name(new_resname));
	
	shaped_graph_->var_assignment_tracker.remove_tracked(
		this, variable_->rescls, old_resname);
	internal_recompute_allowed_names();
	shaped_graph_->var_assignment_tracker.add_tracked(
		this, variable_->rescls, new_resname);
	
	for (const auto & part : interference) {
		jive_shaped_variable * other = part.shaped_variable;
		other->deny_name(new_resname);
		other->sub_squeeze(new_resname->resource_class);
	}
}

bool
jive_shaped_variable::allowed_resource_name(const jive_resource_name * name) const noexcept
{
	return allowed_names_.find(name) != allowed_names_.end();
}

size_t
jive_shaped_variable::allowed_resource_name_count() const noexcept
{
	return allowed_names_.size();
}

void
jive_shaped_variable::initial_assign_gate(jive::gate * gate)
{
	/* during initial build of shaped_graph, other_shape might be NULL */

	for (auto i = gate->interference.begin(); i != gate->interference.end(); i++) {
		jive::gate * other_gate = i->gate;
		jive_variable * other = other_gate->variable;
		if (!other) continue;
		jive_shaped_variable * other_shape = shaped_graph().map_variable(other);
		if (other_shape) {
			jive_shaped_variable::interference_add(this, other_shape);
		}
	}
}

void
jive_shaped_variable::assign_gate(jive::gate * gate)
{
	for (auto i = gate->interference.begin(); i != gate->interference.end(); i++) {
		jive::gate * other_gate = i->gate;
		jive_variable * other = other_gate->variable;
		if (!other) continue;
		jive_shaped_variable * other_shape = shaped_graph().map_variable(other);
		jive_shaped_variable::interference_add(this, other_shape);
	}
}

void
jive_shaped_variable::unassign_gate(jive::gate * gate)
{
	for (auto i = gate->interference.begin(); i != gate->interference.end(); i++) {
		jive::gate * other_gate = i->gate;
		jive_variable * other = other_gate->variable;
		if (!other) continue;
		jive_shaped_variable * other_shape = shaped_graph().map_variable(other);
		jive_shaped_variable::interference_remove(this, other_shape);
	}
}

size_t
jive_shaped_variable::is_active_before(const jive_shaped_node * shaped_node) const noexcept
{
	size_t count = 0;
	jive_ssavar * ssavar;
	JIVE_LIST_ITERATE(variable_->ssavars, ssavar, variable_ssavar_list) {
		count += shaped_graph_->map_ssavar(ssavar)->is_active_before(shaped_node);
	}
	
	return count;
}

size_t
jive_shaped_variable::is_crossing(const jive_shaped_node * shaped_node) const noexcept
{
	size_t count = 0;
	jive_ssavar * ssavar;
	JIVE_LIST_ITERATE(variable_->ssavars, ssavar, variable_ssavar_list) {
		count += shaped_graph_->map_ssavar(ssavar)->is_crossing(shaped_node);
	}
	
	return count;
}

size_t
jive_shaped_variable::is_active_after(const jive_shaped_node * shaped_node) const noexcept
{
	size_t count = 0;
	jive_ssavar * ssavar;
	JIVE_LIST_ITERATE(variable_->ssavars, ssavar, variable_ssavar_list) {
		count += shaped_graph_->map_ssavar(ssavar)->is_active_after(shaped_node);
	}
	
	return count;
}

void
jive_shaped_variable::get_cross_count(jive_resource_class_count * counts) const
{
	counts->clear();

	jive_ssavar * ssavar;
	JIVE_LIST_ITERATE(variable_->ssavars, ssavar, variable_ssavar_list) {
		jive_shaped_ssavar * shaped_ssavar = shaped_graph_->map_ssavar(ssavar);
		
		for (const jive_nodevar_xpoint & xpoint : shaped_ssavar->node_xpoints_) {
			if (xpoint.before_count()) {
				counts->update_union(xpoint.shaped_node().use_count_before());
			}
			if (xpoint.after_count()) {
				counts->update_union(xpoint.shaped_node().use_count_after());
			}
		}
	}
}

void
jive_shaped_variable::recompute_allowed_names()
{
	shaped_graph().var_assignment_tracker.remove_tracked(
		this, variable()->rescls, variable()->resname);
	internal_recompute_allowed_names();
	shaped_graph().var_assignment_tracker.add_tracked(
		this, variable()->rescls, variable()->resname);
}

void
jive_shaped_variable::add_squeeze(const jive_resource_class * rescls)
{
	if (variable()->resname || !variable()->rescls->limit || !rescls->limit) {
		return;
	}
	shaped_graph().var_assignment_tracker.remove_tracked(
		this, variable()->rescls, variable()->resname);
	if (jive_resource_class_intersection(variable()->rescls, rescls)) {
		squeeze_ ++;
	}
	shaped_graph().var_assignment_tracker.add_tracked(
		this, variable()->rescls, variable()->resname);
}

void
jive_shaped_variable::sub_squeeze(const jive_resource_class * rescls)
{
	if (variable()->resname || !variable()->rescls->limit || !rescls->limit) {
		return;
	}
	shaped_graph().var_assignment_tracker.remove_tracked(
		this, variable()->rescls, variable()->resname);
	if (jive_resource_class_intersection(variable()->rescls, rescls)) {
		JIVE_DEBUG_ASSERT(squeeze_ > 0);
		squeeze_ --;
	}
	shaped_graph().var_assignment_tracker.add_tracked(
		this, variable()->rescls, variable()->resname);
}

size_t
jive_shaped_variable::interference_add(
	jive_shaped_variable * first,
	jive_shaped_variable * second)
{
	interference_item * i;
	auto iter = first->interference.find(second);
	if (iter != first->interference.end()) {
		i = iter->whole;
	} else {
		const jive_resource_name * first_name = first->variable()->resname;
		const jive_resource_name * second_name = second->variable()->resname;
		
		if (second_name) {
			first->deny_name(second_name);
		}
		if (first_name) {
			second->deny_name(first_name);
		}
		
		if (!second_name) {
			first->add_squeeze(second->variable()->rescls);
		}
		if (!first_name) {
			second->add_squeeze(first->variable()->rescls);
		}
		
		i = jive_variable_interference_create(first, second);
	}
	return i->count ++;
}

size_t
jive_shaped_variable::interference_remove(
	jive_shaped_variable * first, jive_shaped_variable * second)
{
	auto iter = first->interference.find(second);
	interference_item * i = iter->whole;
	size_t count = -- (i->count);
	if (!i->count) {
		jive_variable_interference_destroy(i);
		const jive_resource_name * first_name = first->variable()->resname;
		const jive_resource_name * second_name = second->variable()->resname;
		
		if (first_name || second_name) {
			first->recompute_allowed_names();
			second->recompute_allowed_names();
		} else {
			first->sub_squeeze(second->variable()->rescls);
			second->sub_squeeze(first->variable()->rescls);
		}
	}
	return count;
}

void
jive_shaped_variable::deny_name(const jive_resource_name * resname)
{
	auto i = allowed_names_.find(resname);
	if (i != allowed_names_.end()) {
		shaped_graph_->var_assignment_tracker.remove_tracked(
			this, variable_->rescls, variable_->resname);
		allowed_names_.erase(i);
		shaped_graph_->var_assignment_tracker.add_tracked(
			this, variable_->rescls, variable_->resname);
	}
}

size_t
jive_shaped_variable::interferes_with(
	const jive_shaped_variable * other) const noexcept
{
	auto i = interference.find(other);
	return i != interference.end() ?  i->whole->count : 0;
}

bool
jive_shaped_variable::can_merge(const jive_variable * other) const noexcept
{
	if (!other) {
		return true;
	}
	
	jive_shaped_variable * other_shape = shaped_graph_->map_variable(other);
	if (other_shape && interferes_with(other_shape)) {
		return false;
	}
	
	const jive_resource_class * new_rescls =
		jive_resource_class_intersection(variable_->rescls, other->rescls);
	if (!new_rescls) {
		return false;
	}
	
	if (check_change_resource_class(new_rescls)) {
		return false;
	}
	
	if (other_shape && other_shape->check_change_resource_class(new_rescls)) {
		return false;
	}
	
	if (variable_->resname && other->resname && variable_->resname != other->resname) {
		return false;
	}
	
	return true;
}

const jive_resource_class *
jive_shaped_variable::check_change_resource_class(
	const jive_resource_class * new_rescls) const noexcept
{
	const jive_resource_class * old_rescls = variable_->rescls;
	if (old_rescls == new_rescls) {
		return nullptr;
	}
	
	jive_ssavar * ssavar;
	JIVE_LIST_ITERATE(variable_->ssavars, ssavar, variable_ssavar_list) {
		const jive_resource_class * overflow = shaped_graph_->map_ssavar(ssavar)
			->check_change_resource_class(old_rescls, new_rescls);
		if (overflow) {
			return overflow;
		}
	}
	
	jive_resource_class_count use_count;
	
	jive::gate * gate;
	JIVE_LIST_ITERATE(variable_->gates, gate, variable_gate_list) {
		jive::input * input;
		JIVE_LIST_ITERATE(gate->inputs, input, gate_inputs_list) {
			jive_node_get_use_count_input(input->node(), &use_count);
			const jive_resource_class * overflow =
				use_count.check_change(old_rescls, new_rescls);
			if (overflow) {
				return overflow;
			}
		}
		jive::output * output;
		JIVE_LIST_ITERATE(gate->outputs, output, gate_outputs_list) {
			jive_node_get_use_count_output(output->node(), &use_count);
			const jive_resource_class * overflow =
				use_count.check_change(old_rescls, new_rescls);
			if (overflow) {
				return overflow;
			}
		}
	}
	
	return nullptr;
}

/* jive_shaped_ssavar */

jive_shaped_ssavar::~jive_shaped_ssavar()
{
}

void
jive_shaped_ssavar::set_boundary_region_depth(size_t depth)
{
	if (boundary_region_depth_ == depth) {
		return;
	}
		
	/* if node is shaped, boundary region is ignored */
	jive_shaped_node * origin_shaped_node = shaped_graph_->map_node_location(
		ssavar_->origin->node());
	if (origin_shaped_node) {
		boundary_region_depth_ = depth;
		return;
	}
	
	xpoints_unregister_arcs();
	boundary_region_depth_ = depth;
	xpoints_register_arcs();
}

size_t
jive_shaped_ssavar::is_active_before(const jive_shaped_node * shaped_node) const noexcept
{
	auto i = node_xpoints_.find(shaped_node);
	return i != node_xpoints_.end() ? i->before_count() : 0;
}

size_t
jive_shaped_ssavar::is_crossing(const jive_shaped_node * shaped_node) const noexcept
{
	auto i = node_xpoints_.find(shaped_node);
	return i != node_xpoints_.end() ? i->cross_count() : 0;
}

size_t
jive_shaped_ssavar::is_active_after(const jive_shaped_node * shaped_node) const noexcept
{
	auto i = node_xpoints_.find(shaped_node);
	return i != node_xpoints_.end() ? i->after_count() : 0;
}

void
jive_shaped_ssavar::common_xpoints_register_arc(
	jive_shaped_node * origin_shaped_node,
	jive_shaped_node * target_shaped_node,
	jive_variable * variable)
{
	jive_crossing_arc_iterator i;
	jive_crossing_arc_iterator_init_ssavar(&i, origin_shaped_node, target_shaped_node, this);
	
	while (i.region) {
		if (i.node) {
			i.node->add_ssavar_crossed(this, variable, 1);
		} else {
			i.region->add_active_top(this, 1);
		}
		jive_crossing_arc_iterator_next(&i);
	}
}

void
jive_shaped_ssavar::common_xpoints_unregister_arc(
	jive_shaped_node * origin_shaped_node,
	jive_shaped_node * target_shaped_node,
	jive_variable * variable) noexcept
{
	jive_crossing_arc_iterator i;
	jive_crossing_arc_iterator_init_ssavar(&i, origin_shaped_node, target_shaped_node, this);
	
	while (i.region) {
		if (i.node) {
			i.node->remove_ssavar_crossed(this, variable, 1);
		} else {
			i.region->remove_active_top(this, 1);
		}
		jive_crossing_arc_iterator_next(&i);
	}
}

void
jive_shaped_ssavar::xpoints_register_arc(
	jive_shaped_node * origin_shaped_node,
	jive_shaped_node * target_shaped_node)
{
	jive_variable * variable = ssavar().variable;

	common_xpoints_register_arc(origin_shaped_node, target_shaped_node, variable);

	if (target_shaped_node &&
			(origin_shaped_node ||
			boundary_region_depth() <= target_shaped_node->node()->region->depth())) {
		target_shaped_node->add_ssavar_before(this, variable, 1);
	}
	
	if (origin_shaped_node && target_shaped_node) {
		origin_shaped_node->add_ssavar_after(this, variable, 1);
	}
}

void
jive_shaped_ssavar::xpoints_unregister_arc(
	jive_shaped_node * origin_shaped_node,
	jive_shaped_node * target_shaped_node) noexcept
{
	jive_variable * variable = ssavar().variable;

	common_xpoints_unregister_arc(origin_shaped_node, target_shaped_node, variable);

	if (target_shaped_node &&
			(origin_shaped_node ||
			boundary_region_depth() <= target_shaped_node->node()->region->depth())) {
		target_shaped_node->remove_ssavar_before(this, variable, 1);
	}

	if (origin_shaped_node && target_shaped_node) {
		origin_shaped_node->remove_ssavar_after(this, variable, 1);
	}
}

void
jive_shaped_ssavar::xpoints_register_region_arc(
	jive_shaped_node * origin_shaped_node,
	jive_shaped_node * target_shaped_node)
{
	jive_variable * variable = ssavar().variable;

	common_xpoints_register_arc(origin_shaped_node, target_shaped_node, variable);

	if (target_shaped_node) {
		target_shaped_node->add_ssavar_before(this, variable, 1);
	}
	
	if (origin_shaped_node && target_shaped_node) {
		origin_shaped_node->add_ssavar_after(this, variable, 1);
	}
}

void
jive_shaped_ssavar::xpoints_unregister_region_arc(
	jive_shaped_node * origin_shaped_node,
	jive_shaped_node * target_shaped_node) noexcept
{
	jive_variable * variable = ssavar().variable;

	common_xpoints_unregister_arc(origin_shaped_node, target_shaped_node, variable);

	if (target_shaped_node) {
		target_shaped_node->remove_ssavar_before(this, variable, 1);
	}
	if (origin_shaped_node && target_shaped_node) {
		origin_shaped_node->remove_ssavar_after(this, variable, 1);
	}
}

void
jive_shaped_ssavar::xpoints_register_arcs()
{
	jive_variable * variable = ssavar().variable;
	jive::input * input;
	JIVE_LIST_ITERATE(ssavar().assigned_inputs, input, ssavar_input_list) {
		xpoints_register_arc(
			shaped_graph().map_node_location(input->origin()->node()),
			shaped_graph().map_node_location(input->node()));
	}
	if (ssavar().assigned_output) {
		jive_shaped_node * origin_shaped_node = shaped_graph().map_node_location(
			ssavar().assigned_output->node());
		if (origin_shaped_node) {
			origin_shaped_node->add_ssavar_after(this, variable, 1);
		}
	}

	for (const jive_region_ssavar_use & use : ssavar().assigned_regions) {
		xpoints_register_region_arc(
			shaped_graph().map_node_location(ssavar().origin->node()),
			shaped_graph().map_node_location(jive_region_get_bottom_node(use.region)));
	}
}

void
jive_shaped_ssavar::xpoints_unregister_arcs() noexcept
{
	jive_variable * variable = ssavar().variable;
	jive::input * input;
	JIVE_LIST_ITERATE(ssavar().assigned_inputs, input, ssavar_input_list) {
		xpoints_unregister_arc(
			shaped_graph().map_node_location(input->origin()->node()),
			shaped_graph().map_node_location(input->node()));
	}
	if (ssavar().assigned_output) {
		jive_shaped_node * origin_shaped_node = shaped_graph().map_node_location(
			ssavar().assigned_output->node());
		if (origin_shaped_node) {
			origin_shaped_node->remove_ssavar_after(this, variable, 1);
		}
	}

	for (const jive_region_ssavar_use & use : ssavar().assigned_regions) {
		xpoints_unregister_region_arc(
			shaped_graph().map_node_location(ssavar().origin->node()),
			shaped_graph().map_node_location(jive_region_get_bottom_node(use.region)));
	}
}

void
jive_shaped_ssavar::xpoints_variable_change(
	jive_variable * old_variable,
	jive_variable * new_variable)
{
	jive_shaped_variable * old_shaped_var = shaped_graph().map_variable(old_variable);
	jive_shaped_variable * new_shaped_var = shaped_graph().map_variable(new_variable);
	const jive_resource_class * old_rescls = jive_variable_get_resource_class(old_variable);
	const jive_resource_class * new_rescls = jive_variable_get_resource_class(new_variable);
	
	for (const jive_nodevar_xpoint & xpoint : node_xpoints_) {
		jive_shaped_node & shaped_node = xpoint.shaped_node();
		
		for (const jive_nodevar_xpoint & other_xpoint : shaped_node.ssavar_xpoints_) {
			if (&other_xpoint == &xpoint) continue;
			jive_shaped_variable * other_shaped_var;
			other_shaped_var = shaped_graph().map_variable(
				other_xpoint.shaped_ssavar().ssavar().variable);
			
			if (xpoint.before_count() && other_xpoint.before_count()) {
				jive_shaped_variable::interference_remove(old_shaped_var, other_shaped_var);
				jive_shaped_variable::interference_add(new_shaped_var, other_shaped_var);
				
			}
			if (xpoint.after_count() && other_xpoint.after_count()) {
				jive_shaped_variable::interference_remove(old_shaped_var, other_shaped_var);
				jive_shaped_variable::interference_add(new_shaped_var, other_shaped_var);
			}
		}
		if (old_rescls != new_rescls) {
			if (xpoint.before_count()) {
				shaped_node.use_count_before_.change(old_rescls, new_rescls);
			}
			if (xpoint.after_count()) {
				shaped_node.use_count_after_.change(old_rescls, new_rescls);
			}
		}
	}
}

void
jive_shaped_ssavar::notify_divert_origin(
	jive::output * old_origin,
	jive::output * new_origin)
{
	jive_shaped_node * old_origin_shaped_node = shaped_graph().map_node_location(
		old_origin->node());
	jive_shaped_node * new_origin_shaped_node = shaped_graph().map_node_location(
		new_origin->node());
	
	jive_variable * variable = ssavar_->variable;
	jive::input * input;
	
	if (old_origin_shaped_node && ssavar_->assigned_output) {
		old_origin_shaped_node->remove_ssavar_after(this, variable, 1);
	}
	JIVE_LIST_ITERATE(ssavar_->assigned_inputs, input, ssavar_input_list) {
		xpoints_unregister_arc(
			shaped_graph().map_node_location(old_origin->node()),
			shaped_graph().map_node_location(input->node()));
	}
	for (const jive_region_ssavar_use & use : ssavar_->assigned_regions) {
		xpoints_unregister_region_arc(
			shaped_graph().map_node_location(old_origin->node()),
			shaped_graph().map_node_location(jive_region_get_bottom_node(use.region)));
	}
	
	JIVE_LIST_ITERATE(ssavar_->assigned_inputs, input, ssavar_input_list) {
		xpoints_register_arc(
			shaped_graph().map_node_location(new_origin->node()),
			shaped_graph().map_node_location(input->node()));
	}
	for (const jive_region_ssavar_use & use : ssavar_->assigned_regions) {
		xpoints_unregister_region_arc(
			shaped_graph().map_node_location(new_origin->node()),
			shaped_graph().map_node_location(jive_region_get_bottom_node(use.region)));
	}

	if (new_origin_shaped_node && ssavar_->assigned_output) {
		new_origin_shaped_node->add_ssavar_after(this, variable, 1);
	}
}

void
jive_shaped_ssavar::xpoints_change_resource_class(
	const jive_resource_class * old_rescls,
	const jive_resource_class * new_rescls)
{
	for (const jive_nodevar_xpoint & xpoint : node_xpoints_) {
		jive_shaped_node & shaped_node = xpoint.shaped_node();
		if (xpoint.before_count()) {
			shaped_node.use_count_before_.change(
				old_rescls, new_rescls);
		}
		if (xpoint.after_count()) {
			shaped_node.use_count_after_.change(
				old_rescls, new_rescls);
		}
	}
}

const jive_resource_class *
jive_shaped_ssavar::check_change_resource_class(
	const jive_resource_class * old_rescls,
	const jive_resource_class * new_rescls) const noexcept
{
	const jive_resource_class * overflow;
	for (const jive_nodevar_xpoint & xpoint : node_xpoints_) {
		jive_shaped_node & shaped_node = xpoint.shaped_node();
		if (xpoint.before_count()) {
			overflow = shaped_node.use_count_before_.check_change(
				old_rescls, new_rescls);
			if (overflow) {
				return overflow;
			}
		}
		if (xpoint.after_count()) {
			overflow = shaped_node.use_count_after().check_change(
				old_rescls, new_rescls);
			if (overflow) {
				return overflow;
			}
		}
	}
	
	return 0;
}
