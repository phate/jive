/*
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/union/unntype.h>
#include <jive/util/buffer.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>

#include <string.h>

namespace jive {
namespace unn {

/* type */

type::~type() noexcept {}

type::type(const jive::unn::declaration * decl) noexcept
	: jive::value::type()
	, decl_(decl)
{}

void
type::label(jive_buffer & buffer) const
{
	jive_buffer_putstr(&buffer, "unn");
}

bool
type::operator==(const jive::base::type & _other) const noexcept
{
	const jive::unn::type * other = dynamic_cast<const jive::unn::type*>(&_other);
	return other != nullptr && this->declaration() == other->declaration();
}

jive::unn::type *
type::copy() const
{
	return new jive::unn::type(this->declaration());
}

jive::input *
type::create_input(jive_node * node, size_t index, jive::output * origin) const
{
	return new jive::unn::input(declaration(), node, index, origin);
}

jive::output *
type::create_output(jive_node * node, size_t index) const
{
	return new jive::unn::output(this->declaration(), node, index);
}

jive_gate *
type::create_gate(jive_graph * graph, const char * name) const
{
	return new jive::unn::gate(this->declaration(), graph, name);
}

/* input */

input::input(const jive::unn::declaration * decl, struct jive_node * node, size_t index,
	jive::output * origin)
	: jive::value::input(node, index, origin)
	, type_(decl)
{}

input::~input() noexcept {}

/* output */

output::output(const jive::unn::declaration * decl, jive_node * node, size_t index)
	: jive::value::output(node, index)
	, type_(decl)
{}

output::~output() noexcept {}

/* gate */

gate::gate(const jive::unn::declaration * decl, jive_graph * graph, const char name[])
	: jive::value::gate(graph, name)
	, type_(decl)
{}

gate::~gate() noexcept {}

}
}
