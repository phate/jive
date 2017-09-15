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
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/substitution.h>
#include <jive/vsdg/traverser.h>

/* graph */

namespace jive {

graph::~graph()
{
	delete root_;

	while (gates.first)
		delete gates.first;
}

graph::graph()
	: normalized_(false)
	, root_(new jive::region(nullptr, this))
{
	gates.first = gates.last = 0;
}

std::unique_ptr<jive::graph>
graph::copy() const
{
	jive::substitution_map smap;
	std::unique_ptr<jive::graph> graph(new jive::graph());
	root()->copy(graph->root(), smap, true, true);
	return graph;
}

jive::node_normal_form *
graph::node_normal_form(const std::type_info & type) noexcept
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
graph::has_active_traversers() const noexcept
{
	for (const auto & slot : tracker_slots) {
		if (slot.in_use)
			return true;
	}

	return false;
}

jive::gate *
graph::create_gate(const std::string & name, const jive::base::type & type)
{
	return new jive::gate(this, name, type);
}

jive::gate *
graph::create_gate(const std::string & name, const jive::resource_class * rescls)
{
	return new jive::gate(this, name, rescls);
}

}

jive_tracker_slot
jive_graph_reserve_tracker_slot_slow(jive::graph * self)
{
	size_t n = self->tracker_slots.size();

	self->tracker_slots.resize(self->tracker_slots.size()+1);

	self->tracker_slots[n].slot.index = n;
	self->tracker_slots[n].slot.cookie = 1;
	self->tracker_slots[n].in_use = true;

	return self->tracker_slots[n].slot;
}
