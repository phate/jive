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

/* immediate_input */

jive_immediate_input::~jive_immediate_input() noexcept {}

jive_immediate_input::jive_immediate_input(struct jive_node * node, size_t index,
	jive_output * origin)
	: jive_value_input(node, index, origin)
{}

/* immediate_output inheritable members */

jive_immediate_output::~jive_immediate_output() noexcept {}

jive_immediate_output::jive_immediate_output(jive_node * node, size_t index)
	: jive_value_output(node, index)
{}

/* immediate_gate inheritable members */

jive_immediate_gate::~jive_immediate_gate() noexcept {}

jive_immediate_gate::jive_immediate_gate(jive_graph * graph, const char name[])
	: jive_value_gate(graph, name)
{}

/* immediate type */

jive_immediate_type::~jive_immediate_type() noexcept {}

void
jive_immediate_type::label(jive_buffer & buffer) const
{
	jive_buffer_putstr(&buffer, "imm");
}

bool
jive_immediate_type::operator==(const jive_type & other) const noexcept
{
	return dynamic_cast<const jive_immediate_type*>(&other) != nullptr;
}

jive_immediate_type *
jive_immediate_type::copy() const
{
	return new jive_immediate_type();
}

jive_input *
jive_immediate_type::create_input(jive_node * node, size_t index, jive_output * origin) const
{
	return new jive_immediate_input(node, index, origin);
}

jive_output *
jive_immediate_type::create_output(jive_node * node, size_t index) const
{
	return new jive_immediate_output(node, index);
}

jive_gate *
jive_immediate_type::create_gate(jive_graph * graph, const char * name) const
{
	return new jive_immediate_gate(graph, name);
}
