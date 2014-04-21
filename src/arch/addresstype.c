/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/addresstype.h>

#include <string.h>

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

static void
jive_address_input_fini_(jive_input * self);
static const jive_type *
jive_address_input_get_type_(const jive_input * self);

static void
jive_address_output_init_(jive_address_output * self, const jive_address_type * type,
	jive_node * ndoe, size_t index);
static void
jive_address_output_fini_(jive_output * self);
static const jive_type *
jive_address_output_get_type_(const jive_output * self);

static void
jive_address_gate_init_(jive_address_gate * self, const jive_address_type * type,
	jive_graph * graph, const char name[]);
static void
jive_address_gate_fini_(jive_gate * self);
static const jive_type *
jive_address_gate_get_type_(const jive_gate * self);

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

const jive_input_class JIVE_ADDRESS_INPUT = {
	parent : &JIVE_VALUE_INPUT,
	fini : jive_address_input_fini_, /* override */
	get_label : jive_input_get_label_, /* inherit */
	get_type : jive_address_input_get_type_ /* override */
};

const jive_output_class JIVE_ADDRESS_OUTPUT = {
	parent : &JIVE_VALUE_OUTPUT,
	fini : jive_address_output_fini_, /* override */
	get_label : jive_output_get_label_, /* inherit */
	get_type : jive_address_output_get_type_ /* override */
};

const jive_gate_class JIVE_ADDRESS_GATE = {
	parent : &JIVE_VALUE_GATE,
	fini : jive_address_gate_fini_, /* inherit */
	get_label : jive_gate_get_label_, /* inherit */
	get_type : jive_address_gate_get_type_ /* override */
};

/* address_type inheritable members */

jive_address_type::~jive_address_type() noexcept {}

jive_address_type::jive_address_type() noexcept
	: jive_value_type(&JIVE_ADDRESS_TYPE)
{}

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
	: jive_value_input(&JIVE_ADDRESS_INPUT, node, index, origin)
{}

void
jive_address_input_fini_(jive_input * self_)
{
	jive_address_input * self = (jive_address_input *) self_;

	jive_input_fini_(self);
}

const jive_type *
jive_address_input_get_type_(const jive_input * self_)
{
	const jive_address_input * self = (const jive_address_input *)self_;
	return &self->type();
}

/* address_output inheritable members */

jive_address_output::~jive_address_output() noexcept {}

jive_address_output::jive_address_output(jive_node * node, size_t index)
	: jive_value_output(&JIVE_ADDRESS_OUTPUT, node, index)
{}

void
jive_address_output_fini_(jive_output * self_)
{
}

const jive_type *
jive_address_output_get_type_(const jive_output * self_)
{
	const jive_address_output * self = (const jive_address_output *)self_;

	return &self->type();
}

/*address_gate inheritable members */

jive_address_gate::~jive_address_gate() noexcept {}

jive_address_gate::jive_address_gate(jive_graph * graph, const char name[])
	: jive_value_gate(&JIVE_ADDRESS_GATE, graph, name)
{}

void jive_address_gate_fini_(jive_gate * self_)
{
	jive_address_gate * self = (jive_address_gate *)self_;

	jive_gate_fini_(self);
}

const jive_type *
jive_address_gate_get_type_(const jive_gate * self_)
{
	const jive_address_gate * self = (const jive_address_gate *) self_;

	return &self->type();
}
