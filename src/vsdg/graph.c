/*
 * Copyright 2010 2011 2012 2013 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2016 Nico Reißmann <nico.reissmann@gmail.com>
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
#include <jive/vsdg/region.h>
#include <jive/vsdg/substitution.h>
#include <jive/vsdg/traverser.h>

/* graph tail node */

namespace jive {

graph_tail_operation::~graph_tail_operation() noexcept
{
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

static void
prune_regions_recursive(jive::region * region)
{
	jive::region * subregion, * next;
	JIVE_LIST_ITERATE_SAFE(region->subregions, subregion, next, region_subregions_list)
		prune_regions_recursive(subregion);
	if (region->nodes.empty())
		delete region;
}

/* graph */

jive_graph::~jive_graph()
{
	delete root()->bottom();
	jive_graph_prune(this);

	prune_regions_recursive(root());

	while (gates.first)
		delete gates.first;
}

jive_graph::jive_graph()
	: root_(new jive::region(nullptr, this))
{
	bottom.first = bottom.last = 0;
	gates.first = gates.last = 0;
	resources_fully_assigned = false;
	normalized = true;
	jive::graph_tail_operation().create_node(root(), {});
}

std::unique_ptr<jive_graph>
jive_graph::copy() const
{
	std::unique_ptr<jive_graph> graph(new jive_graph());

	jive::substitution_map smap;
	jive_region_copy_substitute(root(), graph->root(), smap, false, false);

	return graph;
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
		
		if (node != self->root()->bottom()) {
			delete node;
			if (!next) next = self->bottom.first;
		}
		
		node = next;
	}
	
	jive::region * subregion, * next;
	JIVE_LIST_ITERATE_SAFE(self->root()->subregions, subregion, next, region_subregions_list)
		prune_regions_recursive(subregion);
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
jive_graph_normalize(jive_graph * self)
{
	for (jive_node * node : jive::topdown_traverser(self)) {
		jive::node_normal_form * nf = jive_graph_get_nodeclass_form(
			self, typeid(node->operation()));
		nf->normalize_node(node);
	}
	
	self->normalized = true;
}

jive::gate *
jive_graph_create_gate(jive_graph * self, const std::string & name, const jive::base::type & type)
{
	return new jive::gate(self, name.c_str(), type);
}
