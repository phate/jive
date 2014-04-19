/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/controltype.h>

#include <string.h>

#include <jive/util/list.h>
#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/statetype-private.h>

static jive_input *
jive_control_type_create_input_(const jive_type * self, jive_node * node, size_t index, jive_output * initial_operand);

static jive_output *
jive_control_type_create_output_(const jive_type * self, jive_node * node, size_t index);

static jive_gate *
jive_control_type_create_gate_(const jive_type * self, jive_graph * graph, const char * name);

static jive_type *
jive_control_type_copy_(const jive_type * self);

static void
jive_control_input_init_(jive_control_input * self, jive_node * node, size_t index, jive_output * origin);

static const jive_type *
jive_control_input_get_type_(const jive_input * self);

static void
jive_control_output_init_(jive_control_output * self, jive_node * node, size_t index);

static const jive_type *
jive_control_output_get_type_(const jive_output * self);

static void
jive_control_gate_init_(jive_control_gate * self, struct jive_graph * graph, const char name[]);

static const jive_type *
jive_control_gate_get_type_(const jive_gate * self);

const jive_type_class JIVE_CONTROL_TYPE = {
	parent : &JIVE_STATE_TYPE,
	name : "ctl",
	fini : jive_type_fini_, /* inherit */
	get_label : jive_type_get_label_, /* inherit */
	create_input : jive_control_type_create_input_, /* override */
	create_output : jive_control_type_create_output_, /* override */
	create_gate : jive_control_type_create_gate_, /* override */
	equals : jive_type_equals_, /* inherit */
	copy : jive_control_type_copy_ /* override */
};

const jive_input_class JIVE_CONTROL_INPUT = {
	parent : &JIVE_STATE_INPUT,
	fini : jive_input_fini_, /* inherit */
	get_label : jive_input_get_label_, /* inherit */
	get_type : jive_control_input_get_type_, /* override */
};

const jive_output_class JIVE_CONTROL_OUTPUT = {
	parent : &JIVE_STATE_OUTPUT,
	fini : jive_output_fini_, /* inherit */
	get_label : jive_output_get_label_, /* inherit */
	get_type : jive_control_output_get_type_, /* override */
};

const jive_gate_class JIVE_CONTROL_GATE = {
	parent : &JIVE_STATE_GATE,
	fini : jive_gate_fini_, /* inherit */
	get_label : jive_gate_get_label_, /* inherit */
	get_type : jive_control_gate_get_type_, /* override */
};

static jive_input *
jive_control_type_create_input_(const jive_type * self, jive_node * node, size_t index, jive_output * initial_operand)
{
	jive_control_input * input = new jive_control_input;
	input->class_ = &JIVE_CONTROL_INPUT;
	jive_control_input_init_(input, node, index, initial_operand);
	return input;
}

static jive_output *
jive_control_type_create_output_(const jive_type * self, jive_node * node, size_t index)
{
	jive_control_output * output = new jive_control_output;
	output->class_ = &JIVE_CONTROL_OUTPUT;
	output->active = true;
	jive_control_output_init_(output, node, index);
	return output;
}

static jive_gate *
jive_control_type_create_gate_(const jive_type * self, struct jive_graph * graph, const char * name)
{
	jive_control_gate * gate = jive_context_malloc(graph->context, sizeof(*gate));
	gate->class_ = &JIVE_CONTROL_GATE;
	jive_control_gate_init_(gate, graph, name);
	return gate;
}

static jive_type *
jive_control_type_copy_(const jive_type * self)
{
	jive_control_type * other = new jive_control_type;
	other->class_ = &JIVE_CONTROL_TYPE;
	return other;
}

static void
jive_control_input_init_(jive_control_input * self, jive_node * node, size_t index, jive_output * origin)
{
	jive_state_input_init_(self, node, index, origin);
}

static const jive_type *
jive_control_input_get_type_(const jive_input * self)
{
	static jive_control_type control_type; control_type.class_ = &JIVE_CONTROL_TYPE;
	return &control_type;
}

static void
jive_control_output_init_(jive_control_output * self, jive_node * node, size_t index)
{
	jive_state_output_init_(self, node, index);
}

static const jive_type *
jive_control_output_get_type_(const jive_output * self)
{
	static jive_control_type control_type; control_type.class_ = &JIVE_CONTROL_TYPE;
	return &control_type;
}

static void
jive_control_gate_init_(jive_control_gate * self, struct jive_graph * graph, const char * name)
{
	jive_state_gate_init_(self, graph, name);
}

static const jive_type *
jive_control_gate_get_type_(const jive_gate * self)
{
	static jive_control_type control_type; control_type.class_ = &JIVE_CONTROL_TYPE;
	return &control_type;
}
