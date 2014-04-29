/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/real/rltype.h>
#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/valuetype-private.h>

void
jive_real_gate_init_(jive_real_gate * self, struct jive_graph * graph, const char name[]);

void
jive_real_output_init_(jive_real_output * self, struct jive_node * node, size_t index);

/* real type */

jive_real_type::~jive_real_type() noexcept {}

jive_real_type::jive_real_type() noexcept
	: jive_value_type(&JIVE_REAL_TYPE)
{}

jive_input *
jive_real_type_create_input_(const jive_type * self_, struct jive_node * node, size_t index,
	jive_output * initial_operand)
{
	jive_real_input * input = new jive_real_input(node, index, initial_operand);
	return input;
}

jive_output *
jive_real_type_create_output_(const jive_type * self_, struct jive_node * node, size_t index)
{
	return new jive_real_output(node, index);
}

jive_gate *
jive_real_type_create_gate_(const jive_type * self_, struct jive_graph * graph, const char * name)
{
	return new jive_real_gate(graph, name);
}

jive_type *
jive_real_type_copy_(const jive_type * self_)
{
	const jive_real_type * self = (const jive_real_type *) self_;
	jive_real_type * type = new jive_real_type;
	*type = *self;
	return type;
}

static inline void
jive_real_type_init_(jive_real_type * self)
{
	self->class_ = &JIVE_REAL_TYPE;
}

const jive_type_class JIVE_REAL_TYPE = {
	parent : &JIVE_VALUE_TYPE,
	name : "rl",
	fini : jive_value_type_fini_, /* inherit */
	get_label : jive_type_get_label_, /* inherit */
	create_input : jive_real_type_create_input_, /* override */
	.create_output  = jive_real_type_create_output_, /* override */
	create_gate : jive_real_type_create_gate_, /* override */
	equals : jive_type_equals_, /* inherit */
	copy : jive_real_type_copy_, /* override */
};

/* real input */

jive_real_input::~jive_real_input() noexcept {}

jive_real_input::jive_real_input(struct jive_node * node, size_t index,
	jive_output * origin)
	: jive_value_input(node, index, origin)
{}

/* real output */

jive_real_output::~jive_real_output() noexcept {}

jive_real_output::jive_real_output(struct jive_node * node, size_t index)
	: jive_value_output(node, index)
{}

/* real gate */

jive_real_gate::~jive_real_gate() noexcept {}

jive_real_gate::jive_real_gate(jive_graph * graph, const char name[])
	: jive_value_gate(graph, name)
{}
