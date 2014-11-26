/*
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/float/flttype.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>

#include <string.h>

namespace jive {
namespace flt {

/* float */

type::~type() noexcept {}

jive::flt::type *
type::copy() const
{
	return new jive::flt::type();
}

std::string
type::debug_string() const
{
	return "flt";
}

bool
type::operator==(const jive::base::type & other) const noexcept
{
	return dynamic_cast<const jive::flt::type*>(&other) != nullptr;
}

jive::input *
type::create_input(jive_node * node, size_t index, jive::output * origin) const
{
	return new jive::flt::input(node, index, origin);
}

jive::output *
type::create_output(jive_node * node, size_t index) const
{
	return new jive::flt::output(node, index);
}

jive::gate *
type::create_gate(jive_graph * graph, const char * name) const
{
	return new jive::flt::gate(graph, name);
}

/* input */

input::~input() noexcept {}

input::input(struct jive_node * node, size_t index, jive::output * origin)
	: jive::value::input(node, index, origin)
{}

/* output */

output::~output() noexcept {}

output::output(struct jive_node * node, size_t index)
	: jive::value::output(node, index)
{}

/* gate */

gate::~gate() noexcept {}

gate::gate(jive_graph * graph, const char name[])
	: jive::value::gate(graph, name)
{}

}
}
