/*
 * Copyright 2010 2011 2012 2013 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <cxxabi.h>

#include <jive/rvsdg/graph.h>
#include <jive/rvsdg/label.h>
#include <jive/rvsdg/node-normal-form.h>
#include <jive/rvsdg/node.h>
#include <jive/rvsdg/region.h>
#include <jive/rvsdg/substitution.h>
#include <jive/rvsdg/tracker.h>
#include <jive/types/record.h>
#include <jive/types/union.h>

/* graph */

namespace jive {

graph::~graph()
{
	JIVE_DEBUG_ASSERT(!has_active_trackers(this));

	delete root_;

	while (gates.first())
		delete gates.first();

	unregister_rcddeclarations(this);
	unregister_unndeclarations(this);
}

graph::graph()
	: normalized_(false)
	, root_(new jive::region(nullptr, this))
{}

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

}
