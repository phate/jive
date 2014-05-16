/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/controltype.h>

#include <string.h>

#include <jive/util/buffer.h>
#include <jive/util/list.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>

jive_control_type::~jive_control_type() noexcept {}

void
jive_control_type::label(jive_buffer & buffer) const
{
	jive_buffer_putstr(&buffer, "ctl");
}

bool
jive_control_type::operator==(const jive_type & other) const noexcept
{
	return dynamic_cast<const jive_control_type*>(&other) != nullptr;
}

jive_control_type *
jive_control_type::copy() const
{
	return new jive_control_type();
}

jive_input *
jive_control_type::create_input(jive_node * node, size_t index, jive_output * origin) const
{
	return new jive_control_input(node, index, origin);
}

jive_output *
jive_control_type::create_output(jive_node * node, size_t index) const
{
	return new jive_control_output(true, node, index);
}

jive_gate *
jive_control_type::create_gate(jive_graph * graph, const char * name) const
{
	return new jive_control_gate(graph, name);
}

jive_control_input::~jive_control_input() noexcept {}

jive_control_input::jive_control_input(struct jive_node * node, size_t index,
	jive_output * initial_operand)
	: jive_state_input(node, index, initial_operand)
{}

jive_control_output::~jive_control_output() noexcept {}

jive_control_output::jive_control_output(bool active, struct jive_node * node,
	size_t index)
	: jive_state_output(node, index)
	, active_(active)
{}

jive_control_gate::~jive_control_gate() noexcept {}

jive_control_gate::jive_control_gate(jive_graph * graph, const char name[])
	: jive_state_gate(graph, name)
{}
