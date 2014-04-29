/*
 * Copyright 2013 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/immediate-type.h>

#include <string.h>

#include <jive/util/buffer.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/valuetype-private.h>

/* immediate_input */

jive_immediate_input::~jive_immediate_input() noexcept {}

jive_immediate_input::jive_immediate_input(struct jive_node * node, size_t index,
	jive_output * origin)
	: jive_value_input(node, index, origin)
{}

/* immediate_output inheritable members */

jive_immediate_output::~jive_immediate_output() noexcept {}

jive_immediate_output::jive_immediate_output(jive_node * node, size_t index)
	: jive_value_output(node, index)
{}

/* immediate_gate inheritable members */

jive_immediate_gate::~jive_immediate_gate() noexcept {}

jive_immediate_gate::jive_immediate_gate(jive_graph * graph, const char name[])
	: jive_value_gate(&JIVE_IMMEDIATE_GATE, graph, name)
{}

static const jive_type *
jive_immediate_gate_get_type_(const jive_gate * self_)
{
	const jive_immediate_gate * self = (const jive_immediate_gate *) self_;
	return &self->type();
}

const jive_gate_class JIVE_IMMEDIATE_GATE = {
	parent : &JIVE_VALUE_GATE,
	fini : jive_gate_fini_, /* inherit */
	get_label : jive_gate_get_label_, /* inherit */
	get_type : jive_immediate_gate_get_type_, /* override */
};

/* immediate type */

jive_immediate_type::~jive_immediate_type() noexcept {}

jive_immediate_type::jive_immediate_type() noexcept
	: jive_value_type(&JIVE_IMMEDIATE_TYPE)
{}

static void
jive_immediate_type_fini_( jive_type* self_ )
{
	jive_immediate_type* self = (jive_immediate_type*) self_ ;

	jive_value_type_fini_(self);
}

static void
jive_immediate_type_get_label_(const jive_type * self_, struct jive_buffer * buffer)
{
	jive_buffer_putstr(buffer, "immediate");
}

static jive_input *
jive_immediate_type_create_input_(const jive_type * self_, struct jive_node * node, size_t index,
	jive_output * initial_operand)
{
	return new jive_immediate_input(node, index, initial_operand);
}

static jive_output *
jive_immediate_type_create_output_(const jive_type * self_, struct jive_node * node, size_t index)
{
	return new jive_immediate_output(node, index);
}

static jive_gate *
jive_immediate_type_create_gate_(const jive_type * self_, struct jive_graph * graph, const char * name)
{
	return new jive_immediate_gate(graph, name);
}

static jive_type *
jive_immediate_type_copy_(const jive_type * self_)
{
	const jive_immediate_type * self = (const jive_immediate_type *) self_;
	
	jive_immediate_type * type = new jive_immediate_type;
	
	*type = *self;
	
	return type;
}

const jive_type_class JIVE_IMMEDIATE_TYPE = {
	parent : &JIVE_VALUE_TYPE,
	name : "JIVE_IMMEDIATE_TYPE",
	fini : jive_immediate_type_fini_, /* override */
	get_label : jive_immediate_type_get_label_, /* override */
	create_input : jive_immediate_type_create_input_, /* override */
	create_output : jive_immediate_type_create_output_, /* override */
	create_gate : jive_immediate_type_create_gate_, /* override */
	equals : jive_type_equals_, /* inherit */
	copy : jive_immediate_type_copy_, /* override */
};
