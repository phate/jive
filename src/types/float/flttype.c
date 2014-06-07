/*
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
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

void
type::label(jive_buffer & buffer) const
{
	jive_buffer_putstr(&buffer, "flt");
}

bool
type::operator==(const jive::base::type & other) const noexcept
{
	return dynamic_cast<const jive::flt::type*>(&other) != nullptr;
}

jive::input *
type::create_input(jive_node * node, size_t index, jive_output * origin) const
{
	return new jive::flt::input(node, index, origin);
}

jive_output *
type::create_output(jive_node * node, size_t index) const
{
	return new jive::flt::output(node, index);
}

jive_gate *
type::create_gate(jive_graph * graph, const char * name) const
{
	return new jive::flt::gate(graph, name);
}

/* input */

input::~input() noexcept {}

input::input(struct jive_node * node, size_t index, jive_output * origin)
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
