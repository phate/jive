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

jive_anchor_type::~jive_anchor_type() noexcept {}

void
jive_anchor_type::label(jive_buffer & buffer) const
{
	jive_buffer_putstr(&buffer, "X");
}

bool
jive_anchor_type::operator==(const jive_type & other) const noexcept
{
	return dynamic_cast<const jive_anchor_type*>(&other) != nullptr;
}

jive_anchor_type *
jive_anchor_type::copy() const
{
	return new jive_anchor_type();
}

jive_input *
jive_anchor_type::create_input(jive_node * node, size_t index, jive_output * origin) const
{
	return new jive_anchor_input(node, index, origin);
}

jive_output *
jive_anchor_type::create_output(jive_node * node, size_t index) const
{
	return new jive_anchor_output(node, index);
}

jive_gate *
jive_anchor_type::create_gate(jive_graph * graph, const char * name) const
{
	/*
		FIXME: this is an ugly solution
	*/
	return nullptr;
}

jive_anchor_input::jive_anchor_input(struct jive_node * node, size_t index,
	jive_output * origin)
	: jive_input(node, index, origin)
{
	JIVE_DEBUG_ASSERT(origin->node()->region->anchor == nullptr);
	origin->node()->region->anchor = this;
}

jive_anchor_input::~jive_anchor_input() noexcept
{
	if (origin()->node()->region->anchor == this)
		origin()->node()->region->anchor = nullptr;
}

jive_anchor_output::~jive_anchor_output() noexcept {}

jive_anchor_output::jive_anchor_output(struct jive_node * node, size_t index)
	: jive_output(node, index)
{}
