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
jive_real_input_init_(jive_real_input * self, struct jive_node * node, size_t index,
	jive_output * origin);

void
jive_real_gate_init_(jive_real_gate * self, struct jive_graph * graph, const char name[]);

void
jive_real_output_init_(jive_real_output * self, struct jive_node * node, size_t index);

/* real type */

jive_input *
jive_real_type_create_input_(const jive_type * self_, struct jive_node * node, size_t index,
	jive_output * initial_operand)
{
	jive_real_input * input = jive_context_malloc(node->graph->context, sizeof(*input));
	input->class_ = &JIVE_REAL_INPUT;
	jive_real_input_init_(input, node, index, initial_operand);
	return input;
}

jive_output *
jive_real_type_create_output_(const jive_type * self_, struct jive_node * node, size_t index)
{
	jive_real_output * output = jive_context_malloc(node->graph->context, sizeof(*output));
	output->class_ = &JIVE_REAL_OUTPUT;
	jive_real_output_init_(output, node, index);
	return output;
}

jive_gate *
jive_real_type_create_gate_(const jive_type * self_, struct jive_graph * graph, const char * name)
{
	jive_real_gate * gate = jive_context_malloc(graph->context, sizeof(*gate));
	gate->base.base.class_ = &JIVE_REAL_GATE;
	jive_real_gate_init_(gate, graph, name);
	return &gate->base.base;
}

jive_type *
jive_real_type_copy_(const jive_type * self_, jive_context * context)
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

void
jive_real_input_init_(jive_real_input * self, struct jive_node * node, size_t index,
	jive_output * origin)
{
	jive_value_input_init_(self, node, index, origin);
	jive_real_type_init_(&self->type);
}

const jive_type *
jive_real_input_get_type_(const jive_input * self_)
{
	const jive_real_input * self = (const jive_real_input *) self_;
	return &self->type;
}

const jive_input_class JIVE_REAL_INPUT = {
	parent : &JIVE_VALUE_INPUT,
	fini : jive_input_fini_, /* inherit */
	get_label : jive_input_get_label_, /* inherit */
	get_type : jive_real_input_get_type_, /* override */
};

/* real output */

void
jive_real_output_init_(jive_real_output * self, struct jive_node * node, size_t index)
{
	self->class_ = &JIVE_REAL_OUTPUT;
	jive_value_output_init_(self, node, index);
	jive_real_type_init_(&self->type);
}

const jive_type *
jive_real_output_get_type_(const jive_output * self_)
{
	const jive_real_output * self = (const jive_real_output *) self_;
	return &self->type;
}

const jive_output_class JIVE_REAL_OUTPUT = {
	parent : &JIVE_VALUE_OUTPUT,
	fini : jive_output_fini_, /* inherit */
	get_label : jive_output_get_label_, /* inherit */
	get_type : jive_real_output_get_type_, /* override */
};

/* real gate */

void
jive_real_gate_init_(jive_real_gate * self, struct jive_graph * graph, const char name[])
{
	self->base.base.class_ = &JIVE_REAL_GATE;
	jive_value_gate_init_(&self->base, graph, name);
	jive_real_type_init_(&self->type);
}

const jive_type *
jive_real_gate_get_type_(const jive_gate * self_)
{
	const jive_real_gate * self = (const jive_real_gate *) self_;
	return &self->type;
}

const jive_gate_class JIVE_REAL_GATE = {
	parent : &JIVE_VALUE_GATE,
	fini : jive_gate_fini_, /* inherit */
	get_label : jive_gate_get_label_, /* inherit */
	get_type : jive_real_gate_get_type_, /* override */
};
