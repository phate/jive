/*
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/union/unntype.h>
#include <jive/util/buffer.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>

#include <string.h>

/*union_type inheritable members*/

jive_union_type::~jive_union_type() noexcept {}

jive_union_type::jive_union_type(const jive_union_declaration * decl) noexcept
	: jive_value_type()
	, decl_(decl)
{}

void
jive_union_type::label(jive_buffer & buffer) const
{
	jive_buffer_putstr(&buffer, "unn");
}

bool
jive_union_type::operator==(const jive_type & _other) const noexcept
{
	const jive_union_type * other = dynamic_cast<const jive_union_type*>(&_other);
	return other != nullptr && this->declaration() == other->declaration();
}

jive_union_type *
jive_union_type::copy() const
{
	return new jive_union_type(this->declaration());
}

jive_input *
jive_union_type::create_input(jive_node * node, size_t index, jive_output * origin) const
{
	return new jive_union_input(declaration(), node, index, origin);
}

jive_output *
jive_union_type::create_output(jive_node * node, size_t index) const
{
	return new jive_union_output(this->declaration(), node, index);
}

jive_gate *
jive_union_type::create_gate(jive_graph * graph, const char * name) const
{
	return new jive_union_gate(this->declaration(), graph, name);
}

/* union_input inheritable members */

jive_union_input::jive_union_input(const jive_union_declaration * decl, struct jive_node * node,
	size_t index, jive_output * origin)
	: jive_value_input(node, index, origin)
	, type_(decl)
{}

jive_union_input::~jive_union_input() noexcept
{}

/* union_output inheritable members */

jive_union_output::jive_union_output(const jive_union_declaration * decl, jive_node * node,
	size_t index)
	: jive_value_output(node, index)
	, type_(decl)
{}

jive_union_output::~jive_union_output() noexcept
{}

/* union_gate inheritable members */

jive_union_gate::jive_union_gate(const jive_union_declaration * decl, jive_graph * graph,
	const char name[])
	: jive_value_gate(graph, name)
	, type_(decl)
{}

jive_union_gate::~jive_union_gate() noexcept {}
