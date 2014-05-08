/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/util/buffer.h>
#include <jive/util/list.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/anchortype-private.h>
#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>

#include <jive/vsdg/node.h>
#include <jive/vsdg/graph.h>

const jive_type_class JIVE_ANCHOR_TYPE = {
	parent : &JIVE_TYPE,
	name : "X",
	fini : jive_type_fini_, /* inherit */
	get_label : jive_type_get_label_, /* inherit */
	create_input : jive_anchor_type_create_input_, /* override */
	create_output : jive_anchor_type_create_output_, /* override */
	create_gate : jive_type_create_gate_, /* inherit */
	equals : jive_type_equals_, /* inherit */
	copy : jive_type_copy_ /* inherit */
};

jive_anchor_type::~jive_anchor_type() noexcept {}

jive_anchor_type::jive_anchor_type() noexcept
	: jive_type(&JIVE_ANCHOR_TYPE)
{}

void
jive_anchor_type::label(jive_buffer & buffer) const
{
	jive_buffer_putstr(&buffer, "X");
}

jive_input *
jive_anchor_type_create_input_(const jive_type * self, struct jive_node * node, size_t index, jive_output * initial_operand)
{
	return new jive_anchor_input(node, index, initial_operand);
}

jive_output *
jive_anchor_type_create_output_(const jive_type * self, struct jive_node * node, size_t index)
{
	jive_anchor_output * output = new jive_anchor_output(node, index);
	return output;
}

jive_anchor_input::jive_anchor_input(struct jive_node * node, size_t index,
	jive_output * origin)
	: jive_input(node, index, origin)
{
	JIVE_DEBUG_ASSERT(origin->node->region->anchor == nullptr);
	origin->node->region->anchor = this;
}

jive_anchor_input::~jive_anchor_input() noexcept
{
	if (origin()->node->region->anchor == this)
		origin()->node->region->anchor = nullptr;
}

jive_anchor_output::~jive_anchor_output() noexcept {}

jive_anchor_output::jive_anchor_output(struct jive_node * node, size_t index)
	: jive_output(node, index)
{}
