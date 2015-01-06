/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/controltype.h>

#include <string.h>

#include <jive/util/buffer.h>
#include <jive/util/list.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace ctl {

/* type */

type::~type() noexcept {}

std::string
type::debug_string() const
{
	return "ctl";
}

bool
type::operator==(const jive::base::type & other) const noexcept
{
	return dynamic_cast<const jive::ctl::type*>(&other) != nullptr;
}

jive::ctl::type *
type::copy() const
{
	return new jive::ctl::type();
}

jive::input *
type::create_input(jive_node * node, size_t index, jive::output * origin) const
{
	return new jive::ctl::input(node, index, origin);
}

jive::output *
type::create_output(jive_node * node, size_t index) const
{
	return new jive::ctl::output(node, index);
}

jive::gate *
type::create_gate(jive_graph * graph, const char * name) const
{
	return new jive::ctl::gate(graph, name);
}

/* input */

input::~input() noexcept {}

input::input(struct jive_node * node, size_t index, jive::output * initial_operand)
	: jive::state::input(node, index, initial_operand)
{}

/* output */

output::~output() noexcept {}

output::output(struct jive_node * node, size_t index)
	: jive::state::output(node, index)
{}

/* gate */

gate::~gate() noexcept {}

gate::gate(jive_graph * graph, const char name[])
	: jive::state::gate(graph, name)
{}

}
}
