/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/util/buffer.h>
#include <jive/util/list.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>

#include <jive/vsdg/node.h>
#include <jive/vsdg/graph.h>

namespace jive {
namespace achr {

/* type */

type::~type() noexcept {}

void
type::label(jive_buffer & buffer) const
{
	jive_buffer_putstr(&buffer, "X");
}

bool
type::operator==(const jive::base::type & other) const noexcept
{
	return dynamic_cast<const jive::achr::type*>(&other) != nullptr;
}

jive::achr::type *
type::copy() const
{
	return new jive::achr::type();
}

jive_input *
type::create_input(jive_node * node, size_t index, jive_output * origin) const
{
	return new jive::achr::input(node, index, origin);
}

jive_output *
type::create_output(jive_node * node, size_t index) const
{
	return new jive::achr::output(node, index);
}

jive_gate *
type::create_gate(jive_graph * graph, const char * name) const
{
	/*
		FIXME: this is an ugly solution
	*/
	return nullptr;
}

/* input */

input::input(struct jive_node * node, size_t index, jive_output * origin)
	: jive_input(node, index, origin)
{
	JIVE_DEBUG_ASSERT(origin->node()->region->anchor == nullptr);
	origin->node()->region->anchor = this;
}

input::~input() noexcept
{
	if (origin()->node()->region->anchor == this)
		origin()->node()->region->anchor = nullptr;
}

/* output */

output::~output() noexcept {}

output::output(struct jive_node * node, size_t index)
	: jive_output(node, index)
{}

}
}
