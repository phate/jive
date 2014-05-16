/*
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/float/flttype.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>

#include <string.h>

/* float_type inheritable members */

jive_float_type::~jive_float_type() noexcept {}

jive_float_type *
jive_float_type::copy() const
{
	return new jive_float_type();
}

void
jive_float_type::label(jive_buffer & buffer) const
{
	jive_buffer_putstr(&buffer, "flt");
}

bool
jive_float_type::operator==(const jive_type & other) const noexcept
{
	return dynamic_cast<const jive_float_type*>(&other) != nullptr;
}

jive_input *
jive_float_type::create_input(jive_node * node, size_t index, jive_output * origin) const
{
	return new jive_float_input(node, index, origin);
}

jive_output *
jive_float_type::create_output(jive_node * node, size_t index) const
{
	return new jive_float_output(node, index);
}

jive_gate *
jive_float_type::create_gate(jive_graph * graph, const char * name) const
{
	return new jive_float_gate(graph, name);
}

/* float_input inheritable members */

jive_float_input::~jive_float_input() noexcept {}

jive_float_input::jive_float_input(struct jive_node * node, size_t index,
	jive_output * origin)
	: jive_value_input(node, index, origin)
{}

/* float_output inheritable members */

jive_float_output::~jive_float_output() noexcept {}

jive_float_output::jive_float_output(struct jive_node * node, size_t index)
	: jive_value_output(node, index)
{}

/* float_gate inheritable members */

jive_float_gate::~jive_float_gate() noexcept {}

jive_float_gate::jive_float_gate(jive_graph * graph, const char name[])
	: jive_value_gate(graph, name)
{}
