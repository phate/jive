/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/memorytype.h>

#include <string.h>

#include <jive/context.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/graph.h>

jive_memory_input::~jive_memory_input() noexcept {}

jive_memory_input::jive_memory_input(struct jive_node * node, size_t index,
	jive_output * origin)
	: jive_state_input(node, index, origin)
{}

jive_memory_output::~jive_memory_output() noexcept {}

jive_memory_output::jive_memory_output(jive_node * node, size_t index)
	: jive_state_output(node, index)
{}

jive_memory_gate::~jive_memory_gate() noexcept {}

jive_memory_gate::jive_memory_gate(jive_graph * graph, const char name[])
	: jive_state_gate(graph, name)
{}

jive_memory_type::~jive_memory_type() noexcept {}

void
jive_memory_type::label(jive_buffer & buffer) const
{
	jive_buffer_putstr(&buffer, "mem");
}

bool
jive_memory_type::operator==(const jive_type & other) const noexcept
{
	return dynamic_cast<const jive_memory_type*>(&other) != nullptr;
}

jive_memory_type *
jive_memory_type::copy() const
{
	return new jive_memory_type();
}

jive_input *
jive_memory_type::create_input(jive_node * node, size_t index, jive_output * origin) const
{
	return new jive_memory_input(node, index, origin);
}

jive_output *
jive_memory_type::create_output(jive_node * node, size_t index) const
{
	return new jive_memory_output(node, index);
}

jive_gate *
jive_memory_type::create_gate(jive_graph * graph, const char * name) const
{
	return new jive_memory_gate(graph, name);
}
