/*
 * Copyright 2013 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/immediate-type.h>

#include <string.h>

#include <jive/util/buffer.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace imm {

/* input */

input::~input() noexcept {}

input::input(struct jive_node * node, size_t index, jive::output * origin)
	: jive::value::input(node, index, origin)
{}

/* output */

output::~output() noexcept {}

output::output(jive_node * node, size_t index)
	: jive::value::output(node, index)
{}

/* gate */

gate::~gate() noexcept {}

gate::gate(jive_graph * graph, const char name[])
	: jive::value::gate(graph, name)
{}

/* type */

type::~type() noexcept {}

std::string
type::debug_string() const
{
	return "imm";
}

bool
type::operator==(const jive::base::type & other) const noexcept
{
	return dynamic_cast<const jive::imm::type*>(&other) != nullptr;
}

jive::imm::type *
type::copy() const
{
	return new jive::imm::type();
}

jive::input *
type::create_input(jive_node * node, size_t index, jive::output * origin) const
{
	return new jive::imm::input(node, index, origin);
}

jive::output *
type::create_output(jive_node * node, size_t index) const
{
	return new jive::imm::output(node, index);
}

jive::gate *
type::create_gate(jive_graph * graph, const char * name) const
{
	return new jive::imm::gate(graph, name);
}

}
}
