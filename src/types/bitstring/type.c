/*
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/type.h>
#include <jive/util/buffer.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>

#include <stdio.h>
#include <string.h>

/* bitstring_type inheritable members */

jive_bitstring_type::~jive_bitstring_type() noexcept {}

jive_bitstring_type::jive_bitstring_type(size_t nbits) noexcept
	: jive_value_type()
	, nbits_(nbits)
{}

void
jive_bitstring_type::label(jive_buffer & buffer) const
{
	char tmp[16];
	snprintf(tmp, sizeof(tmp), "bits%zd", nbits());
	jive_buffer_putstr(&buffer, tmp);
}

bool
jive_bitstring_type::operator==(const jive_type & _other) const noexcept
{
	const jive_bitstring_type * other = dynamic_cast<const jive_bitstring_type*>(&_other);
	return other != nullptr && this->nbits() == other->nbits();
}

jive_bitstring_type *
jive_bitstring_type::copy() const
{
	return new jive_bitstring_type(this->nbits());
}

jive_input *
jive_bitstring_type::create_input(jive_node * node, size_t index, jive_output * origin) const
{
	return new jive_bitstring_input(nbits_, node, index, origin);
}

jive_output *
jive_bitstring_type::create_output(jive_node * node, size_t index) const
{
	return new jive_bitstring_output(nbits(), node, index);
}

jive_gate *
jive_bitstring_type::create_gate(jive_graph * graph, const char * name) const
{
	return new jive_bitstring_gate(nbits(), graph, name);
}

/* bitstring_input inheritable members */

jive_bitstring_input::jive_bitstring_input(size_t nbits, struct jive_node * node, size_t index,
	jive_output * origin)
	: jive_value_input(node, index, origin)
	, type_(nbits)
{}

jive_bitstring_input::~jive_bitstring_input() noexcept {}

/* bitstring_output inheritable members */

jive_bitstring_output::jive_bitstring_output(size_t nbits, struct jive_node * node,
	size_t index)
	: jive_value_output(node, index)
	, type_(nbits)
{}

jive_bitstring_output::~jive_bitstring_output() noexcept {}

/* bitstring_gate inheritable members */

jive_bitstring_gate::jive_bitstring_gate(size_t nbits, jive_graph * graph,
	const char name[])
	: jive_value_gate(graph, name)
	, type_(nbits)
{}

jive_bitstring_gate::~jive_bitstring_gate() noexcept {}
