/*
 * Copyright 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/seqtype.h>

namespace jive {
namespace seq {

/* type */

type::~type() noexcept {}

std::string
type::debug_string() const
{
	return "seq";
}

bool
type::operator==(const jive::base::type & other) const noexcept
{
	return dynamic_cast<const jive::seq::type*>(&other) != nullptr;
}

jive::seq::type *
type::copy() const
{
	return new jive::seq::type();
}

jive::input *
type::create_input(jive_node * node, size_t index, jive::output * origin) const
{
	return new jive::seq::input(node, index, origin);
}

jive::output *
type::create_output(jive_node * node, size_t index) const
{
	return new jive::seq::output(node, index);
}

jive::gate *
type::create_gate(jive_graph * graph, const char * name) const
{
	return new jive::seq::gate(graph, name);
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
