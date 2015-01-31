/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/util/list.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace achr {

/* type */

type::~type() noexcept {}

std::string
type::debug_string() const
{
	return "X";
}

bool
type::operator==(const jive::base::type & other) const noexcept
{
	return dynamic_cast<const jive::achr::type*>(&other) != nullptr;
}

jive::achr::type *
type::copy() const
{
	return new jive::achr::type();
}

jive::output *
type::create_output(jive_node * node, size_t index) const
{
	return new jive::achr::output(node, index);
}

jive::gate *
type::create_gate(jive_graph * graph, const char * name) const
{
	/*
		FIXME: this is an ugly solution
	*/
	return nullptr;
}

/* output */

output::~output() noexcept {}

output::output(struct jive_node * node, size_t index)
	: jive::output(node, index, jive::achr::type())
{}

}
}
