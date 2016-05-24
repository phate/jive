/*
 * Copyright 2010 2011 2012 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/regalloc/shaped-node.h>

#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/shaped-region.h>
#include <jive/regalloc/shaped-variable.h>
#include <jive/regalloc/xpoint.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/variable.h>

void
jive_shaped_node::remove_all_crossed() noexcept
{
	auto i = ssavar_xpoints_.begin();
	while (i != ssavar_xpoints_.end()) {
		jive_nodevar_xpoint * xpoint = i.ptr();
		++i;
		remove_ssavar_crossed(
			&xpoint->shaped_ssavar(),
			xpoint->shaped_ssavar().ssavar().variable,
			xpoint->cross_count());
	}
}

void
jive_shaped_node::add_crossings_from_lower_location(
	jive_shaped_graph * shaped_graph,
	jive_shaped_node * lower)
{
	for (const jive_nodevar_xpoint & xpoint : lower->ssavar_xpoints_) {
		if (!xpoint.before_count()) continue;
		const jive_ssavar & ssavar = xpoint.shaped_ssavar().ssavar();
		if (ssavar.origin->node() == node()) {
			add_ssavar_after(&xpoint.shaped_ssavar(), ssavar.variable, xpoint.before_count());
		} else {
			if (dynamic_cast<const jive::achr::type*>(&ssavar.origin->type())) {
				continue;
			}
			if (xpoint.shaped_ssavar().boundary_region_depth() > node()->region->depth() &&
				!shaped_graph->is_node_placed(ssavar.origin->node())) {
				continue;
			}
			add_ssavar_crossed(&xpoint.shaped_ssavar(), ssavar.variable, xpoint.before_count());
		}
	}
	
	for (size_t n = 0; n < lower->node()->ninputs; n++) {
		jive::input * input = lower->node()->inputs[n];
		if (!dynamic_cast<const jive::achr::type*>(&input->type())) continue;
		jive_shaped_region * shaped_region = shaped_graph->map_region(
			input->producer()->region);
		
		/* if this is a control edge, pass through variables from the top
		of the subregion */
		add_crossings_from_lower_location(shaped_graph, shaped_region->first_in_region());
		/* variables passed through both the anchor node and the
		region would be counted twice, so remove the duplicates */
		for (const jive_nodevar_xpoint & xpoint : lower->ssavar_xpoints_) {
			if (xpoint.cross_count_) {
				remove_ssavar_crossed(
					&xpoint.shaped_ssavar(),
					xpoint.shaped_ssavar().ssavar().variable,
					xpoint.cross_count_);
			}
		}
	}
}

jive_shaped_node::~jive_shaped_node() noexcept
{
}

void
jive_shaped_node::add_to_cut(jive_cut * cut, jive_shaped_node * before)
{
	cut_ = cut;

	JIVE_DEBUG_ASSERT(
		!node_->region->anchor ||
		shaped_graph_->is_node_placed(node_->region->anchor->node()));

	/* set aside crossings for ssavars originating here */
	for (size_t n = 0; n < node_->noutputs; n++) {
		jive::output * output = node_->outputs[n];
		jive_ssavar * ssavar;
		
		JIVE_LIST_ITERATE(output->originating_ssavars, ssavar, originating_ssavar_list) {
			jive_shaped_ssavar * shaped_ssavar = shaped_graph().map_ssavar(ssavar);
			
			jive::input * input;
			JIVE_LIST_ITERATE(ssavar->assigned_inputs, input, ssavar_input_list) {
				shaped_ssavar->xpoints_unregister_arc(
					nullptr,
					shaped_graph().map_node_location(input->node()));
			}
			for (const jive_region_ssavar_use & use : ssavar->assigned_regions) {
				shaped_ssavar->xpoints_unregister_region_arc(
					nullptr,
					shaped_graph().map_node_location(jive_region_get_bottom_node(use.region)));
			}
			
			shaped_ssavar->boundary_region_depth_ = (size_t) -1;
		}
	}

	cut->nodes_.insert(
		cut->nodes_.make_element_iterator(before),
		this);

	jive_shaped_node * next = next_in_region();
	if (next) {
		add_crossings_from_lower_location(&shaped_graph(), next);
	} else if (node_->region->anchor) {
		next = shaped_graph().map_node_location(node_->region->anchor->node());
		for (const jive_nodevar_xpoint & xpoint : next->ssavar_xpoints_) {
			add_ssavar_crossed(
				&xpoint.shaped_ssavar(),
				xpoint.shaped_ssavar().ssavar().variable,
				xpoint.cross_count_);
		}
	}
	
	for (size_t n = 0; n < node_->ninputs; n++) {
		jive::input * input = node_->inputs[n];
		if (dynamic_cast<const jive::achr::type*>(&input->type())) {
			for (const jive_nodevar_xpoint & xpoint : ssavar_xpoints_) {
				jive_shaped_region * shaped_region = shaped_graph().map_region(
					input->origin()->node()->region);
				shaped_region->add_active_top(&xpoint.shaped_ssavar(), xpoint.cross_count_);
			}
		}
	}

	/* reinstate crossings for ssavars originating here */
	for (size_t n = 0; n < node_->noutputs; n++) {
		jive::output * output = node_->outputs[n];
		jive_ssavar * ssavar;
		
		JIVE_LIST_ITERATE(output->originating_ssavars, ssavar, originating_ssavar_list) {
			jive_shaped_ssavar * shaped_ssavar = shaped_graph().map_ssavar(ssavar);
			
			jive::input * input;
			JIVE_LIST_ITERATE(ssavar->assigned_inputs, input, ssavar_input_list) {
				shaped_ssavar->xpoints_register_arc(
					this,
					shaped_graph().map_node_location(input->node()));
			}
			for (const jive_region_ssavar_use & use : ssavar->assigned_regions) {
				shaped_ssavar->xpoints_register_region_arc(
					this,
					shaped_graph().map_node_location(jive_region_get_bottom_node(use.region)));
			}
			
			if (ssavar->assigned_output) {
				add_ssavar_after(shaped_ssavar, ssavar->variable, 1);
			}
		}
	}

	/* add crossings for ssavars used here */
	for (size_t n = 0; n < node_->ninputs; n++) {
		jive::input * input = node_->inputs[n];
		if (input->ssavar) {
			shaped_graph().map_ssavar(input->ssavar)->xpoints_register_arc(
				shaped_graph().map_node_location(input->origin()->node()),
				this);
		}
	}

	/* if this is the bottom node of a loop region, need to register
	crossings on behalf of this region */
	if (node_ == jive_region_get_bottom_node(node_->region)) {
		for (const jive_region_ssavar_use & use : node_->region->used_ssavars) {
			shaped_graph().map_ssavar(use.ssavar)->xpoints_register_region_arc(
				shaped_graph().map_node_location(use.ssavar->origin->node()),
				this);
		}
	}
	shaped_graph().on_node_place(this);

}

void
jive_shaped_node::add_ssavar_before(
	jive_shaped_ssavar * shaped_ssavar,
	jive_variable * variable,
	size_t count)
{
	if (count != 0) {
		inc_active_before(get_xpoint(shaped_ssavar), shaped_ssavar, variable, count);
	}
}

void
jive_shaped_node::remove_ssavar_before(
	jive_shaped_ssavar * shaped_ssavar,
	jive_variable * variable,
	size_t count) noexcept
{
	if (count != 0) {
		jive_nodevar_xpoint * xpoint = ssavar_xpoints_.find(shaped_ssavar).ptr();
		dec_active_before(xpoint, shaped_ssavar, variable, count);
		xpoint->put();
	}
}

void
jive_shaped_node::add_ssavar_crossed(
	jive_shaped_ssavar * shaped_ssavar,
	jive_variable * variable,
	size_t count)
{
	if (count != 0) {
		jive_nodevar_xpoint * xpoint = get_xpoint(shaped_ssavar);
		xpoint->cross_count_ += count;
		inc_active_before(xpoint, shaped_ssavar, variable, count);
		inc_active_after(xpoint, shaped_ssavar, variable, count);
	}
}

void
jive_shaped_node::remove_ssavar_crossed(
	jive_shaped_ssavar * shaped_ssavar,
	jive_variable * variable,
	size_t count) noexcept
{
	if (count != 0) {
		jive_nodevar_xpoint * xpoint = ssavar_xpoints_.find(shaped_ssavar).ptr();
		xpoint->cross_count_ -= count;
		dec_active_before(xpoint, shaped_ssavar, variable, count);
		dec_active_after(xpoint, shaped_ssavar, variable, count);
		xpoint->put();
	}
}

void
jive_shaped_node::add_ssavar_after(
	jive_shaped_ssavar * shaped_ssavar,
	jive_variable * variable,
	size_t count)
{
	if (count != 0) {
		inc_active_after(get_xpoint(shaped_ssavar), shaped_ssavar, variable, count);
	}
}

void
jive_shaped_node::remove_ssavar_after(
	jive_shaped_ssavar * shaped_ssavar,
	jive_variable * variable,
	size_t count) noexcept
{
	if (count != 0) {
		jive_nodevar_xpoint * xpoint = ssavar_xpoints_.find(shaped_ssavar).ptr();
		dec_active_after(xpoint, shaped_ssavar, variable, count);
		xpoint->put();
	}
}

void
jive_shaped_node::inc_active_after(
	jive_nodevar_xpoint * xpoint,
	jive_shaped_ssavar * shaped_ssavar,
	jive_variable * variable,
	size_t count)
{
	if (xpoint->after_count_ == 0) {
		for (const jive_nodevar_xpoint & other_xpoint : ssavar_xpoints_) {
			if (!other_xpoint.after_count_) continue;
			if (&other_xpoint == xpoint) continue;
			jive_shaped_variable::interference_add(
				shaped_graph().map_variable(variable),
				shaped_graph().map_variable(other_xpoint.shaped_ssavar().ssavar().variable)
			);
		}
		const jive_resource_class * overflow;
		overflow = use_count_after_.add(jive_variable_get_resource_class(variable));
		JIVE_DEBUG_ASSERT(!overflow);
	}
	xpoint->after_count_ += count;
}

void
jive_shaped_node::dec_active_after(
	jive_nodevar_xpoint * xpoint,
	jive_shaped_ssavar * shaped_ssavar,
	jive_variable * variable,
	size_t count) noexcept
{
	JIVE_DEBUG_ASSERT(xpoint->after_count_ >= count);
	xpoint->after_count_ -= count;
	if (xpoint->after_count_ == 0) {
		for (const jive_nodevar_xpoint & other_xpoint : ssavar_xpoints_) {
			if (!other_xpoint.after_count_) continue;
			if (&other_xpoint == xpoint) continue;
			jive_shaped_variable::interference_remove(
				shaped_graph().map_variable(variable),
				shaped_graph().map_variable(other_xpoint.shaped_ssavar().ssavar().variable)
			);
		}
		use_count_after_.sub(jive_variable_get_resource_class(variable));
	}
}

void
jive_shaped_node::inc_active_before(
	jive_nodevar_xpoint * xpoint,
	jive_shaped_ssavar * shaped_ssavar,
	jive_variable * variable,
	size_t count)
{
	if (xpoint->before_count_ == 0) {
		for (const jive_nodevar_xpoint & other_xpoint : ssavar_xpoints_) {
			if (!other_xpoint.before_count_) continue;
			if (&other_xpoint == xpoint) continue;
			jive_shaped_variable::interference_add(
				shaped_graph().map_variable(variable),
				shaped_graph().map_variable(other_xpoint.shaped_ssavar().ssavar().variable)
			);
		}
		const jive_resource_class * overflow;
		overflow = use_count_before_.add(jive_variable_get_resource_class(variable));
		(void) overflow;
		JIVE_DEBUG_ASSERT(!overflow);
	}
	xpoint->before_count_ += count;
}

void
jive_shaped_node::dec_active_before(
	jive_nodevar_xpoint * xpoint,
	jive_shaped_ssavar * shaped_ssavar,
	jive_variable * variable,
	size_t count) noexcept
{
	JIVE_DEBUG_ASSERT(xpoint->before_count_ >= count);
	xpoint->before_count_ -= count;
	if (xpoint->before_count_ == 0) {
		for (const jive_nodevar_xpoint & other_xpoint : ssavar_xpoints_) {
			if (!other_xpoint.before_count_) continue;
			if (&other_xpoint == xpoint) continue;
			jive_shaped_variable::interference_remove(
				shaped_graph().map_variable(variable),
				shaped_graph().map_variable(other_xpoint.shaped_ssavar().ssavar().variable)
			);
		}
		use_count_before_.sub(jive_variable_get_resource_class(variable));
	}
}

jive_shaped_node *
jive_shaped_node::prev_in_cut() noexcept
{
	auto i = cut_->nodes_.make_element_iterator(this);
	if (i == cut_->nodes_.begin()) {
		return nullptr;
	} else {
		return std::prev(i).ptr();
	}
}

jive_shaped_node *
jive_shaped_node::next_in_cut() noexcept
{
	auto i = std::next(cut_->nodes_.make_element_iterator(this));
	if (i == cut_->nodes_.end()) {
		return nullptr;
	} else {
		return i.ptr();
	}
}

jive_shaped_node *
jive_shaped_node::prev_in_region() noexcept
{
	auto tmp = prev_in_cut();
	if (tmp) { return tmp; }

	jive_shaped_region & region = cut_->shaped_region();
	auto i = region.cuts_.make_element_iterator(cut_);
	while (i != region.cuts_.begin()) {
		--i;
		if (!i->nodes_.empty()) {
			return std::prev(i->nodes_.end()).ptr();
		}
	}

	return nullptr;
}

jive_shaped_node *
jive_shaped_node::next_in_region() noexcept
{
	auto tmp = next_in_cut();
	if (tmp) { return tmp; }

	jive_shaped_region & region = cut_->shaped_region();
	auto i = std::next(region.cuts_.make_element_iterator(cut_));
	while (i != region.cuts_.end()) {
		if (!i->nodes_.empty()) {
			return i->nodes_.begin().ptr();
		}
		++i;
	}

	return nullptr;
}

jive_varcut
jive_shaped_node::get_active_before() const
{
	jive_varcut cut;
	for (const jive_nodevar_xpoint & xpoint : ssavar_xpoints_) {
		if (xpoint.before_count()) {
			cut.ssavar_add(&xpoint.shaped_ssavar(), xpoint.before_count());
		}
	}
	return cut;
}

jive_varcut
jive_shaped_node::get_active_after() const
{
	jive_varcut cut;
	for (const jive_nodevar_xpoint & xpoint : ssavar_xpoints_) {
		if (xpoint.after_count()) {
			cut.ssavar_add(&xpoint.shaped_ssavar(), xpoint.after_count());
		}
	}
	return cut;
}

bool
jive_shaped_node::is_resource_name_active_after(
	const jive_resource_name * name) const noexcept
{
	for (const jive_nodevar_xpoint & xpoint : ssavar_xpoints_) {
		if (xpoint.after_count() &&
			jive_variable_get_resource_name(xpoint.shaped_ssavar().ssavar().variable) == name) {
			return true;
		}
	}
	
	return false;
}

bool
jive_shaped_node::is_resource_name_active_before(
	const jive_resource_name * name) const noexcept
{
	for (const jive_nodevar_xpoint & xpoint : ssavar_xpoints_) {
		if (xpoint.before_count() &&
			jive_variable_get_resource_name(xpoint.shaped_ssavar().ssavar().variable) == name) {
			return true;
		}
	}
	
	return false;
}

void
jive_shaped_node::remove_from_cut()
{
	if (!cut_) {
		return;
	}

	shaped_graph().on_node_deplace(this);

	/* note: need to do this first, before actually removing node from
	 * cut as nodes in subordinate regions need to know location of
	 * anchor */
	for (size_t n = 0; n < node_->ninputs; n++) {
		jive::input * input = node_->inputs[n];
		if (dynamic_cast<const jive::achr::type *>(&input->type())) {
			jive_region * region = input->producer()->region;
			shaped_graph().map_region(region)->clear_cuts();
		}
	}

	/* remove things that cross this node */
	remove_all_crossed();

	/* set aside crossings of vars beginning or ending here */
	for (size_t n = 0; n < node_->ninputs; n++) {
		jive::input * input = node_->inputs[n];
		jive_ssavar * ssavar = input->ssavar;
		if (ssavar) {
			shaped_graph().map_ssavar(ssavar)->xpoints_unregister_arc(
				shaped_graph().map_node_location(input->origin()->node()),
				this);
		}
	}
	for (size_t n = 0; n < node_->noutputs; n++) {
		jive::output * output = node_->outputs[n];
		jive::input * user;
		JIVE_LIST_ITERATE(output->users, user, output_users_list) {
			jive_ssavar * ssavar = user->ssavar;
			if (ssavar) {
				shaped_graph().map_ssavar(ssavar)->xpoints_unregister_arc(
					this,
					shaped_graph().map_node_location(user->node()));
			}
		}
		jive_ssavar * ssavar = output->ssavar;
		if (ssavar) {
			remove_ssavar_after(shaped_graph().map_ssavar(ssavar), ssavar->variable, 1);
		}
	}
	
	/* if this is the bottom node of a loop region, unregister
	crossings on behalf of this region */
	if (node_ == jive_region_get_bottom_node(node_->region)) {
		for (jive_region_ssavar_use & use : node_->region->used_ssavars) {
			shaped_graph().map_ssavar(use.ssavar)->xpoints_unregister_region_arc(
				shaped_graph().map_node_location(use.ssavar->origin->node()),
				this);
		}
	}
	
	cut_->nodes_.erase(this);
	JIVE_DEBUG_ASSERT(ssavar_xpoints_.size() == 0);

	/* reinstate crossings for those arcs that have this node as origin */
	for (size_t n = 0; n < node_->noutputs; n++) {
		jive::output * output = node_->outputs[n];
		jive::input * user;
		JIVE_LIST_ITERATE(output->users, user, output_users_list) {
			jive_ssavar * ssavar = user->ssavar;
			if (ssavar) {
				shaped_graph().map_ssavar(ssavar)->xpoints_register_arc(
					nullptr,
					shaped_graph().map_node_location(user->node()));
			}
		}
	}

	cut_ = nullptr;
}

const jive_shaped_node_downward_iterator &
jive_shaped_node_downward_iterator::operator++()
{
	jive_shaped_node * current = visit_stack_.back();
	visit_stack_.pop_back();
	jive_shaped_node * next = current->next_in_region();
	if (next) {
		visit_stack_.push_back(next);
		jive_node * node = next->node();
		for (size_t n = node->ninputs; n; --n) {
			jive::input * input = node->inputs[n-1];
			if (dynamic_cast<const jive::achr::type*>(&input->type())) {
				jive_shaped_region * sub = shaped_graph_->map_region(
					input->origin()->node()->region);
				jive_shaped_node * first = sub->first_in_region();
				if (first) {
					visit_stack_.push_back(first);
				}
			}
		}
	}
	return *this;
}
