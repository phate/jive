/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/double/dbltype.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/valuetype-private.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>

/* double type */

jive_double_type::~jive_double_type() noexcept {}

jive_double_type::jive_double_type() noexcept
	: jive_value_type(&JIVE_DOUBLE_TYPE)
{}

void
jive_double_type::label(jive_buffer & buffer) const
{
	jive_buffer_putstr(&buffer, "dbl");
}

bool
jive_double_type::operator==(const jive_type & other) const noexcept
{
	return dynamic_cast<const jive_double_type*>(&other) != nullptr;
}

static jive_input *
jive_double_type_create_input_(const jive_type * self_, struct jive_node * node, size_t index,
	jive_output * initial_operand)
{
	return new jive_double_input(node, index, initial_operand);
}

static jive_output *
jive_double_type_create_output_(const jive_type * self_, struct jive_node * node, size_t index)
{
	return new jive_double_output(node, index);
}

static jive_gate *
jive_double_type_create_gate_(const jive_type * self_, struct jive_graph * graph, const char * name)
{
	return new jive_double_gate(graph, name);
}

static jive_type *
jive_double_type_copy_(const jive_type * self_)
{
	const jive_double_type * self = (const jive_double_type *) self_;
	jive_double_type * type = new jive_double_type;
	*type = *self;
	return type;
}

static inline void
jive_double_type_init_(jive_double_type * self)
{
	self->class_ = &JIVE_DOUBLE_TYPE;
}

const jive_type_class JIVE_DOUBLE_TYPE = {
	parent : &JIVE_VALUE_TYPE,
	name : "dbl",
	fini : jive_value_type_fini_, /* inherit */
	get_label : jive_type_get_label_, /* inherit */
	create_input : jive_double_type_create_input_, /* override */
	.create_output  = jive_double_type_create_output_, /* override */
	create_gate : jive_double_type_create_gate_, /* override */
	equals : jive_type_equals_, /* override */
	copy : jive_double_type_copy_, /* override */
};

/* double input */

jive_double_input::~jive_double_input() noexcept {}

jive_double_input::jive_double_input(struct jive_node * node, size_t index,
	jive_output * origin)
	: jive_value_input(node, index, origin)
{}

/* double output */

jive_double_output::~jive_double_output() noexcept {}

jive_double_output::jive_double_output(struct jive_node * node, size_t index)
	: jive_value_output(node, index)
{}

/* double gate */

jive_double_gate::~jive_double_gate() noexcept {}

jive_double_gate::jive_double_gate(jive_graph * graph, const char name[])
	: jive_value_gate(graph, name)
{}
