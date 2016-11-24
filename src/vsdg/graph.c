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
	prune();

	prune_regions_recursive(root());

	while (gates.first)
		delete gates.first;
}

jive_graph::jive_graph()
	: normalized_(false)
	, root_(new jive::region(nullptr, this))
{
	bottom.first = bottom.last = 0;
	gates.first = gates.last = 0;
	jive::graph_tail_operation().create_node(root(), {});
}

std::unique_ptr<jive_graph>
jive_graph::copy() const
{
	jive::substitution_map smap;
	std::unique_ptr<jive_graph> graph(new jive_graph());
	root()->copy(graph->root(), smap, false, false);
	return graph;
}

void
jive_graph::normalize()
{
	for (auto node : jive::topdown_traverser(this))
		node_normal_form(typeid(node->operation()))->normalize_node(node);

	normalized_ = true;
}

jive::node_normal_form *
jive_graph::node_normal_form(const std::type_info & type) noexcept
{
	auto i = node_normal_forms_.find(std::type_index(type));
	if (i != node_normal_forms_.end())
		return i.ptr();

	const auto cinfo = dynamic_cast<const abi::__si_class_type_info *>(&type);
	auto parent_normal_form = cinfo ? node_normal_form(*cinfo->__base_type) : nullptr;

	std::unique_ptr<jive::node_normal_form> nf(
		jive::node_normal_form::create(type, parent_normal_form, this));

	jive::node_normal_form * result = nf.get();
	node_normal_forms_.insert(std::move(nf));

	return result;
}

bool
jive_graph::has_active_traversers() const noexcept
{
	for (const auto & slot : tracker_slots) {
		if (slot.in_use)
			return true;
	}

	return false;
}

jive::gate *
jive_graph::create_gate(const jive::base::type & type, const std::string & name)
{
	return new jive::gate(this, name.c_str(), type);
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

void
jive_graph::prune()
{
	jive::node * node = bottom.first;
	while (node) {
		jive::node * next = node->graph_bottom_list.next;
		
		if (node != root()->bottom()) {
			delete node;

			if (!next)
				next = bottom.first;
		}
		node = next;
	}
	
	jive::region * subregion, * next;
	JIVE_LIST_ITERATE_SAFE(root()->subregions, subregion, next, region_subregions_list)
		prune_regions_recursive(subregion);
}
