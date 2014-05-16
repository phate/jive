/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/addresstype.h>

#include <string.h>

#include <jive/util/buffer.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>

/* address_type inheritable members */

jive_address_type::~jive_address_type() noexcept {}

void
jive_address_type::label(jive_buffer & buffer) const
{
	jive_buffer_putstr(&buffer, "addr");
}

bool
jive_address_type::operator==(const jive_type & other) const noexcept
{
	return dynamic_cast<const jive_address_type*>(&other) != nullptr;
}

jive_address_type *
jive_address_type::copy() const
{
	return new jive_address_type();
}

jive_input *
jive_address_type::create_input(jive_node * node, size_t index, jive_output * origin) const
{
	return new jive_address_input(node, index, origin);
}

jive_output *
jive_address_type::create_output(jive_node * node, size_t index) const
{
	return new jive_address_output(node, index);
}

jive_gate *
jive_address_type::create_gate(jive_graph * graph, const char * name) const
{
	return new jive_address_gate(graph, name);
}

/* address_input inheritable members */

jive_address_input::~jive_address_input() noexcept {}

jive_address_input::jive_address_input(struct jive_node * node, size_t index,
	jive_output * origin)
	: jive_value_input(node, index, origin)
{}

/* address_output inheritable members */

jive_address_output::~jive_address_output() noexcept {}

jive_address_output::jive_address_output(jive_node * node, size_t index)
	: jive_value_output(node, index)
{}

/*address_gate inheritable members */

jive_address_gate::~jive_address_gate() noexcept {}

jive_address_gate::jive_address_gate(jive_graph * graph, const char name[])
	: jive_value_gate(graph, name)
{}
