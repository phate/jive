/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/graph.h>

#include <cxxabi.h>

#include <jive/util/list.h>
#include <jive/vsdg/graph-private.h>
#include <jive/vsdg/label.h>
#include <jive/vsdg/node-normal-form.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region-private.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/substitution.h>
#include <jive/vsdg/traverser.h>
#include <jive/vsdg/variable.h>

/* graph tail node */

namespace jive {

graph_tail_operation::~graph_tail_operation() noexcept
{
}

bool
graph_tail_operation::operator==(const operation & other) const noexcept
{
	return dynamic_cast<const graph_tail_operation *>(&other);
}

size_t
graph_tail_operation::narguments() const noexcept
{
	return 0;
}

const jive::base::type &
graph_tail_operation::argument_type(size_t index) const noexcept
{
	throw std::logic_error("tail node has no arguments");
}

size_t
graph_tail_operation::nresults() const noexcept
{
	return 0;
}

const jive::base::type &
graph_tail_operation::result_type(size_t index) const noexcept
{
	throw std::logic_error("tail node has no results");
}

jive_node *
graph_tail_operation::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	JIVE_DEBUG_ASSERT(region->bottom == NULL);

	return jive_opnode_create(jive::graph_tail_operation(), region, arguments, arguments + narguments);
}

std::string
graph_tail_operation::debug_string() const
{
	return "GRAPH_TAIL";
}

std::unique_ptr<jive::operation>
graph_tail_operation::copy() const
{
	return std::unique_ptr<jive::operation>(new graph_tail_operation(*this));
}

}

/* graph */

static inline void
jive_graph_init_(jive_graph * self)
{
	self->variables.first = self->variables.last = 0;
	self->unused_variables.first = self->unused_variables.last = 0;
	self->bottom.first = self->bottom.last = 0;
	self->gates.first = self->gates.last = 0;
	self->resources_fully_assigned = false;
	self->normalized = true;
	self->floating_region_count = 0;
	
	jive_region_notifier_slot_init(&self->on_region_create);
	jive_region_notifier_slot_init(&self->on_region_destroy);
	jive_region_ssavar_notifier_slot_init(&self->on_region_add_used_ssavar);
	jive_region_ssavar_notifier_slot_init(&self->on_region_remove_used_ssavar);
	
	jive_node_notifier_slot_init(&self->on_node_create);
	jive_node_notifier_slot_init(&self->on_node_destroy);
	jive_node_depth_notifier_slot_init(&self->on_node_depth_change);
	
	jive_input_notifier_slot_init(&self->on_input_create);
	jive_input_change_notifier_slot_init(&self->on_input_change);
	jive_input_notifier_slot_init(&self->on_input_destroy);
	
	jive_output_notifier_slot_init(&self->on_output_create);
	jive_output_notifier_slot_init(&self->on_output_destroy);
	
	jive_variable_notifier_slot_init(&self->on_variable_create);
	jive_variable_notifier_slot_init(&self->on_variable_destroy);
	jive_variable_gate_notifier_slot_init(&self->on_variable_assign_gate);
	jive_variable_gate_notifier_slot_init(&self->on_variable_unassign_gate);
	jive_variable_resource_class_notifier_slot_init(&self->on_variable_resource_class_change);
	jive_variable_resource_name_notifier_slot_init(&self->on_variable_resource_name_change);
	
	jive_gate_notifier_slot_init(&self->on_gate_interference_add);
	jive_gate_notifier_slot_init(&self->on_gate_interference_remove);
	
	jive_ssavar_notifier_slot_init(&self->on_ssavar_create);
	jive_ssavar_notifier_slot_init(&self->on_ssavar_destroy);
	jive_ssavar_input_notifier_slot_init(&self->on_ssavar_assign_input);
	jive_ssavar_input_notifier_slot_init(&self->on_ssavar_unassign_input);
	jive_ssavar_output_notifier_slot_init(&self->on_ssavar_assign_output);
	jive_ssavar_output_notifier_slot_init(&self->on_ssavar_unassign_output);
	jive_ssavar_divert_notifier_slot_init(&self->on_ssavar_divert_origin);
	jive_ssavar_variable_notifier_slot_init(&self->on_ssavar_variable_change);
	
	self->root_region = new jive_region;
	jive_region_init_(self->root_region, self, 0);
	self->root_region->bottom =
		jive::graph_tail_operation().create_node(self->root_region, 0, nullptr);
}

static void
prune_regions_recursive(jive_region * region)
{
	jive_region * subregion, * next;
	JIVE_LIST_ITERATE_SAFE(region->subregions, subregion, next, region_subregions_list)
		prune_regions_recursive(subregion);
	if (jive_region_empty(region)) jive_region_destroy(region);
}

static void
jive_graph_fini_(jive_graph * self)
{
	jive_node_destroy(self->root_region->bottom);
	jive_graph_prune(self);
	
	prune_regions_recursive(self->root_region);
	
	while (self->gates.first)
		delete self->gates.first;
	
	while(self->unused_variables.first) jive_variable_destroy(self->unused_variables.first);
	
	jive_region_notifier_slot_fini(&self->on_region_create);
	jive_region_notifier_slot_fini(&self->on_region_destroy);
	jive_region_ssavar_notifier_slot_fini(&self->on_region_add_used_ssavar);
	jive_region_ssavar_notifier_slot_fini(&self->on_region_remove_used_ssavar);
	
	jive_node_notifier_slot_fini(&self->on_node_create);
	jive_node_notifier_slot_fini(&self->on_node_destroy);
	jive_node_depth_notifier_slot_fini(&self->on_node_depth_change);
	
	jive_input_notifier_slot_fini(&self->on_input_create);
	jive_input_change_notifier_slot_fini(&self->on_input_change);
	jive_input_notifier_slot_fini(&self->on_input_destroy);
	
	jive_output_notifier_slot_fini(&self->on_output_create);
	jive_output_notifier_slot_fini(&self->on_output_destroy);
	
	jive_variable_notifier_slot_fini(&self->on_variable_create);
	jive_variable_notifier_slot_fini(&self->on_variable_destroy);
	jive_variable_gate_notifier_slot_fini(&self->on_variable_assign_gate);
	jive_variable_gate_notifier_slot_fini(&self->on_variable_unassign_gate);
	jive_variable_resource_class_notifier_slot_fini(&self->on_variable_resource_class_change);
	jive_variable_resource_name_notifier_slot_fini(&self->on_variable_resource_name_change);
	
	jive_gate_notifier_slot_fini(&self->on_gate_interference_add);
	jive_gate_notifier_slot_fini(&self->on_gate_interference_remove);
	
	jive_ssavar_notifier_slot_fini(&self->on_ssavar_create);
	jive_ssavar_notifier_slot_fini(&self->on_ssavar_destroy);
	jive_ssavar_input_notifier_slot_fini(&self->on_ssavar_assign_input);
	jive_ssavar_input_notifier_slot_fini(&self->on_ssavar_unassign_input);
	jive_ssavar_output_notifier_slot_fini(&self->on_ssavar_assign_output);
	jive_ssavar_output_notifier_slot_fini(&self->on_ssavar_unassign_output);
	jive_ssavar_divert_notifier_slot_fini(&self->on_ssavar_divert_origin);
	jive_ssavar_variable_notifier_slot_fini(&self->on_ssavar_variable_change);
}

jive_graph *
jive_graph_create()
{
	jive_graph * graph = new jive_graph;
	jive_graph_init_(graph);
	return graph;
}

void
jive_graph_destroy(jive_graph * self)
{
	jive_graph_fini_(self);
	delete self;
}

jive_tracker_slot
jive_graph_reserve_tracker_slot_slow(jive_graph * self)
{
	size_t n = self->tracker_slots.size();

	self->tracker_slots.resize(self->tracker_slots.size()+1);
	
	self->tracker_slots[n].slot.index = n;
	self->tracker_slots[n].slot.cookie = 1;
	self->tracker_slots[n].in_use = true;
	
	return self->tracker_slots[n].slot;
}

bool
jive_graph_has_active_traversers(const jive_graph * self)
{
	for (size_t n = 0; n < self->tracker_slots.size(); n++)
		if (self->tracker_slots[n].in_use) return true;
	return false;
}

void
jive_graph_prune(jive_graph * self)
{
	jive_node * node;
	node = self->bottom.first;
	while(node) {
		jive_node * next = node->graph_bottom_list.next;
		
		if (node != self->root_region->bottom) {
			jive_node_destroy(node);
			if (!next) next = self->bottom.first;
		}
		
		node = next;
	}
	
	jive_region * subregion, * next;
	JIVE_LIST_ITERATE_SAFE(self->root_region->subregions, subregion, next, region_subregions_list)
		prune_regions_recursive(subregion);
}

jive_graph *
jive_graph_copy(jive_graph * self)
{
	jive_graph * new_graph = jive_graph_create();
	
	jive_substitution_map * subst = jive_substitution_map_create();
	jive_region_copy_substitute(self->root_region, new_graph->root_region, subst, false, false);
	jive_substitution_map_destroy(subst);
	
	return new_graph;
}

void
jive_graph_push_outward(jive_graph * self)
{
	jive_traverser * trav = jive_topdown_traverser_create(self);
	
	for(;;) {
		jive_node * node = jive_traverser_next(trav);
		if (!node)
			break;
		
		while (jive_node_can_move_outward(node))
			jive_node_move_outward(node);
	}
	
	jive_traverser_destroy(trav);

#ifdef JIVE_DEBUG
	jive_region_verify_hull(self->root_region);
#endif
}

void
jive_graph_pull_inward(jive_graph * self)
{
	jive_traverser * trav = jive_bottomup_traverser_create(self);
	
	for(;;) {
		jive_node * node = jive_traverser_next(trav);
		if (!node)
			break;
		jive_region * region;
		do {
			region = node->region;
			jive_node_move_inward(node);
		} while (region != node->region);
	}
	
	jive_traverser_destroy(trav);

#ifdef JIVE_DEBUG
	jive_region_verify_hull(self->root_region);
#endif
}

jive::node_normal_form *
jive_graph_get_nodeclass_form(
	jive_graph * self,
	const std::type_info & type)
{
	auto i = self->new_node_normal_forms.find(std::type_index(type));
	if (i != self->new_node_normal_forms.end()) {
		return i.ptr();
	}

	const auto* cinfo = dynamic_cast<const abi::__si_class_type_info *>(&type);
	jive::node_normal_form * parent_normal_form =
		cinfo ? jive_graph_get_nodeclass_form(self, *cinfo->__base_type) : nullptr;

	std::unique_ptr<jive::node_normal_form> nf(jive::node_normal_form::create(
		type, parent_normal_form, self));
	jive::node_normal_form * result = nf.get();
	self->new_node_normal_forms.insert(std::move(nf));
	return result;
}

void
jive_graph_mark_denormalized(jive_graph * self)
{
	self->normalized = false;
}

void
jive_graph_normalize(jive_graph * self)
{
	jive_traverser * trav = jive_topdown_traverser_create(self);
	
	jive_node * node;
	for(node = jive_traverser_next(trav); node; node = jive_traverser_next(trav)) {
		jive::node_normal_form * nf = jive_graph_get_nodeclass_form(
			self, typeid(node->operation()));
		nf->normalize_node(node);
	}
	
	jive_traverser_destroy(trav);
	self->normalized = true;
}
