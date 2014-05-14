/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/addresstype.h>

#include <string.h>

#include <jive/util/buffer.h>
#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/valuetype-private.h>
#include <jive/vsdg/node.h>

static jive_input *
jive_address_type_create_input_(const jive_type * self, jive_node * node,
	size_t index, jive_output * initial_operand);
static jive_output *
jive_address_type_create_output_(const jive_type * self, jive_node * node,
	size_t index);
static jive_gate *
jive_address_type_create_gate_(const jive_type * self, jive_graph * graph,
	const char name[]);
static jive_type *
jive_address_type_copy_(const jive_type * self);

const jive_type_class JIVE_ADDRESS_TYPE = {
	parent : &JIVE_VALUE_TYPE,
	name : "addr",
	fini : jive_value_type_fini_, /* inherit */
	get_label : jive_type_get_label_, /* inherit */
	create_input : jive_address_type_create_input_, /* override */
	create_output : jive_address_type_create_output_, /* override */
	create_gate : jive_address_type_create_gate_, /* override */
	equals : jive_type_equals_, /* inherit */
	copy : jive_address_type_copy_, /* override */
};

/* address_type inheritable members */

jive_address_type::~jive_address_type() noexcept {}

jive_address_type::jive_address_type() noexcept
	: jive_value_type(&JIVE_ADDRESS_TYPE)
{}

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

std::unique_ptr<jive_type>
jive_address_type::copy() const
{
	return std::unique_ptr<jive_type>(new jive_address_type());
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

jive_type *
jive_address_type_copy_(const jive_type * self_)
{
	jive_address_type * type = new jive_address_type;
	return type;
}

jive_input *
jive_address_type_create_input_(const jive_type * self_, jive_node * node,
	size_t index, jive_output * initial_operand)
{
	return new jive_address_input(node, index, initial_operand);
}

jive_output *
jive_address_type_create_output_(const jive_type * self_, jive_node * node, size_t index)
{
	return new jive_address_output(node, index);
}

jive_gate *
jive_address_type_create_gate_(const jive_type * self_, jive_graph * graph,
	const char * name)
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
