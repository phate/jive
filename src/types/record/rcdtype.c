/*
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/record/rcdtype.h>
#include <jive/util/buffer.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>

#include <string.h>

/* record_type inheritable members */

jive_record_type::~jive_record_type() noexcept {}

jive_record_type::jive_record_type(const jive_record_declaration * decl) noexcept
	: jive_value_type()
	, decl_(decl)
{}

void
jive_record_type::label(jive_buffer & buffer) const
{
	jive_buffer_putstr(&buffer, "rcd");
}

bool
jive_record_type::operator==(const jive_type & _other) const noexcept
{
	const jive_record_type * other = dynamic_cast<const jive_record_type*>(&_other);
	return other != nullptr && this->declaration() == other->declaration();
}

jive_record_type *
jive_record_type::copy() const
{
	return new jive_record_type(this->declaration());
}

jive_input *
jive_record_type::create_input(jive_node * node, size_t index, jive_output * origin) const
{
	return new jive_record_input(this->declaration(), node, index, origin);
}

jive_output *
jive_record_type::create_output(jive_node * node, size_t index) const
{
	return new jive_record_output(this->declaration(), node, index);
}

jive_gate *
jive_record_type::create_gate(jive_graph * graph, const char * name) const
{
	return new jive_record_gate(this->declaration(), graph, name);
}

/* record_input inheritable members */

jive_record_input::jive_record_input(const jive_record_declaration * decl, struct jive_node * node,
	size_t index, jive_output * origin)
	: jive_value_input(node, index, origin)
	, type_(decl)
{}

jive_record_input::~jive_record_input() noexcept {}

/* record_output inheritable members */

jive_record_output::jive_record_output(const jive_record_declaration * decl, struct jive_node * node,
	size_t index)
	: jive_value_output(node, index)
	, type_(decl)
{}

jive_record_output::~jive_record_output() noexcept {}

/* record_gate inheritable members */

jive_record_gate::jive_record_gate(const jive_record_declaration * decl, jive_graph * graph,
	const char name[])
	: jive_value_gate(graph, name)
	, type_(decl)
{}

jive_record_gate::~jive_record_gate() noexcept {}
