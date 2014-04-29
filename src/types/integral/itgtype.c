/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/integral/itgtype.h>

#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/valuetype-private.h>

static void
jive_integral_type_fini_(jive_type * self_);

static jive_type *
jive_integral_type_copy_(const jive_type * self_);

static jive_input *
jive_integral_type_create_input_(const jive_type * self_, struct jive_node * node, size_t index,
	jive_output * origin);

static jive_output *
jive_integral_type_create_output_(const jive_type * self_, struct jive_node * node, size_t index);

static jive_gate *
jive_integral_type_create_gate_(const jive_type * self_, struct jive_graph * graph,
	const char * name);

static inline void
jive_integral_gate_init_(jive_integral_gate * self_, struct jive_graph * graph,
	const char * name);

static const jive_type *
jive_integral_gate_get_type_(const jive_gate * self_);

const jive_type_class JIVE_INTEGRAL_TYPE = {
	parent : &JIVE_VALUE_TYPE,
	name : "igt",
	fini : jive_integral_type_fini_, /* inherit */
	get_label : jive_type_get_label_, /* inherit */
	create_input : jive_integral_type_create_input_, /* override */
	create_output : jive_integral_type_create_output_, /* override */
	create_gate : jive_integral_type_create_gate_, /* override */
	equals : jive_type_equals_, /* inherit */
	copy : jive_integral_type_copy_, /* override */
};

const jive_gate_class JIVE_INTEGRAL_GATE = {
	parent : &JIVE_VALUE_GATE,
	fini : jive_gate_fini_, /* inherit */
	get_label : jive_gate_get_label_, /* inherit */
	get_type : jive_integral_gate_get_type_, /* override */
};

/* integral_type members */

jive_integral_type::~jive_integral_type() noexcept {}

jive_integral_type::jive_integral_type() noexcept
	: jive_value_type(&JIVE_INTEGRAL_TYPE)
{}

static void
jive_integral_type_fini_(jive_type * self_)
{
	jive_integral_type * self = (jive_integral_type *) self_;
	jive_value_type_fini_(self);
}

static jive_type *
jive_integral_type_copy_(const jive_type * self_)
{
	const jive_integral_type * self = (const jive_integral_type *) self_;
	jive_integral_type * type = new jive_integral_type;
	type->class_ = &JIVE_INTEGRAL_TYPE;

	return type;
}

static jive_input *
jive_integral_type_create_input_(const jive_type * self_, struct jive_node * node, size_t index,
	jive_output * origin)
{
	return new jive_integral_input(node, index, origin);
}

static jive_output *
jive_integral_type_create_output_(const jive_type * self_, struct jive_node * node, size_t index)
{
	return new jive_integral_output(node, index);
}

static jive_gate *
jive_integral_type_create_gate_(const jive_type * self_, struct jive_graph * graph,
	const char * name)
{
	return new jive_integral_gate(graph, name);
}

/* integral_input members */

jive_integral_input::~jive_integral_input() noexcept {}

jive_integral_input::jive_integral_input(struct jive_node * node, size_t index,
	jive_output * origin)
	: jive_value_input(node, index, origin)
{}

/* integral_output members */

jive_integral_output::~jive_integral_output() noexcept {}

jive_integral_output::jive_integral_output(jive_node * node, size_t index)
	: jive_value_output(node, index)
{}

/* integral_gate members */

jive_integral_gate::~jive_integral_gate() noexcept {}

jive_integral_gate::jive_integral_gate(jive_graph * graph, const char name[])
	: jive_value_gate(&JIVE_INTEGRAL_GATE, graph, name)
{}

static const jive_type *
jive_integral_gate_get_type_(const jive_gate * self_)
{
	const jive_integral_gate * self = (const jive_integral_gate *) self_;

	return &self->type();
}
