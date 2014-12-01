/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/memorytype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>

#include <string.h>

namespace jive {
namespace mem {

/* input */

input::~input() noexcept {}

input::input(struct jive_node * node, size_t index, jive::output * origin)
	: jive::state::input(node, index, origin)
{}

/* output */

output::~output() noexcept {}

output::output(jive_node * node, size_t index)
	: jive::state::output(node, index)
{}

/* gate */

gate::~gate() noexcept {}

gate::gate(jive_graph * graph, const char name[])
	: jive::state::gate(graph, name)
{}

type::~type() noexcept {}

/* type */

std::string
type::debug_string() const
{
	return "mem";
}

bool
type::operator==(const jive::base::type & other) const noexcept
{
	return dynamic_cast<const jive::mem::type*>(&other) != nullptr;
}

jive::mem::type *
type::copy() const
{
	return new jive::mem::type();
}

jive::input *
type::create_input(jive_node * node, size_t index, jive::output * origin) const
{
	return new jive::mem::input(node, index, origin);
}

jive::output *
type::create_output(jive_node * node, size_t index) const
{
	return new jive::mem::output(node, index);
}

jive::gate *
type::create_gate(jive_graph * graph, const char * name) const
{
	return new jive::mem::gate(graph, name);
}

}
}
