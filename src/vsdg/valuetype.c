/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/valuetype.h>
#include <jive/vsdg/valuetype-private.h>
#include <jive/vsdg/graph-private.h>
#include <jive/vsdg/basetype-private.h>

#include <jive/vsdg/node.h>
#include <jive/vsdg/graph.h>

#include <jive/util/list.h>

const jive_type_class JIVE_VALUE_TYPE = {
	parent : &JIVE_TYPE,
	name : "X",
	fini : jive_value_type_fini_, /* override */
	get_label : jive_type_get_label_, /* inherit */
	create_input : jive_value_type_create_input_, /* override */
	create_output : jive_value_type_create_output_, /* override */
	create_gate : jive_value_type_create_gate_, /* override */
	equals : jive_type_equals_, /* inherit */
	copy : jive_value_type_copy_, /* override */
};

const jive_gate_class JIVE_VALUE_GATE = {
	parent : &JIVE_GATE,
	fini : jive_gate_fini_, /* inherit */
	get_label : jive_gate_get_label_, /* inherit */
	get_type : jive_value_gate_get_type_, /* override */
};

jive_value_type::~jive_value_type() noexcept {}

jive_value_type::jive_value_type(const jive_type_class * class_) noexcept
	: jive_type(class_)
{}

void
jive_value_type_fini_(jive_type * self)
{
	jive_type_fini_(self);
}

jive_type *
jive_value_type_copy_(const jive_type * self_)
{
	return nullptr;
} 

jive_input *
jive_value_type_create_input_(const jive_type * self, struct jive_node * node, size_t index, jive_output * initial_operand)
{
	return nullptr;
}

jive_output *
jive_value_type_create_output_(const jive_type * self, struct jive_node * node, size_t index)
{
	return nullptr;
}

jive_gate *
jive_value_type_create_gate_(const jive_type * self, struct jive_graph * graph, const char * name)
{
	return nullptr;
}

/* value inputs */

jive_value_input::~jive_value_input() noexcept {};

jive_value_input::jive_value_input(struct jive_node * node, size_t index,
	jive_output * initial_operand)
	: jive_input(node, index, initial_operand)
{}

/* value outputs */

jive_value_output::~jive_value_output() noexcept {}

jive_value_output::jive_value_output(struct jive_node * node, size_t index)
	: jive_output(node, index)
{}

/* value gates */

jive_value_gate::~jive_value_gate() noexcept {}

jive_value_gate::jive_value_gate(const jive_gate_class * class_, jive_graph * graph,
	const char name[])
	: jive_gate(class_, graph, name)
{}

const jive_type *
jive_value_gate_get_type_(const jive_gate * self)
{
	return nullptr;
}
