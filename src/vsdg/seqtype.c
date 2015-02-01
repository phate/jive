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

jive::gate *
type::create_gate(jive_graph * graph, const char * name) const
{
	return new jive::seq::gate(graph, name);
}

/* output */

output::~output() noexcept {}

output::output(struct jive_node * node, size_t index)
	: jive::state::output(node, index, jive::seq::type())
{}

/* gate */

gate::~gate() noexcept {}

gate::gate(jive_graph * graph, const char name[])
	: jive::state::gate(graph, name)
{}

}
}
