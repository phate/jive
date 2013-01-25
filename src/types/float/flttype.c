/*
 * Copyright 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/float/flttype-private.h>
#include <jive/vsdg/valuetype-private.h>
#include <jive/vsdg/basetype-private.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>

#include <string.h>

const jive_type_class JIVE_FLOAT_TYPE = {
	.parent = &JIVE_VALUE_TYPE,
	.fini = jive_value_type_fini_, /* inherit */
	.get_label = jive_float_type_get_label_, /* override */
	.create_input = jive_float_type_create_input_, /* override */
	.create_output  = jive_float_type_create_output_, /* override */
	.create_gate = jive_float_type_create_gate_, /* override */
	.equals = jive_type_equals_, /* override */
	.copy = jive_float_type_copy_, /* override */ 
};

const jive_input_class JIVE_FLOAT_INPUT = {
	.parent = &JIVE_VALUE_INPUT,
	.fini = jive_input_fini_, /* inherit */
	.get_label = jive_input_get_label_, /* inherit */
	.get_type = jive_float_input_get_type_, /* override */
};

const jive_output_class JIVE_FLOAT_OUTPUT = {
	.parent = &JIVE_VALUE_OUTPUT,
	.fini = jive_output_fini_, /* inherit */
	.get_label = jive_output_get_label_, /* inherit */
	.get_type = jive_float_output_get_type_, /* override */
};

const jive_gate_class JIVE_FLOAT_GATE = {
	.parent = &JIVE_VALUE_GATE,
	.fini = jive_gate_fini_, /* inherit */
	.get_label = jive_gate_get_label_, /* inherit */
	.get_type = jive_float_gate_get_type_, /* override */
};

/* float_type inheritable members */

char *
jive_float_type_get_label_(const jive_type * self_)
{
	return strdup("flt");
}

jive_input *
jive_float_type_create_input_(const jive_type * self_, struct jive_node * node, size_t index,
	jive_output * initial_operand)
{
	jive_float_input * input = jive_context_malloc(node->graph->context, sizeof(*input));
	input->base.base.class_ = &JIVE_FLOAT_INPUT;
	jive_float_input_init_(input, node, index, initial_operand);
	return &input->base.base;
}

jive_output *
jive_float_type_create_output_(const jive_type * self_, struct jive_node * node, size_t index)
{
	jive_float_output * output = jive_context_malloc(node->graph->context, sizeof(*output));
	output->base.base.class_ = &JIVE_FLOAT_OUTPUT;
	jive_float_output_init_(output, node, index);
	return &output->base.base;
}

jive_gate *
jive_float_type_create_gate_(const jive_type * self_, struct jive_graph * graph, const char * name)
{
	jive_float_gate * gate = jive_context_malloc(graph->context, sizeof(*gate));
	gate->base.base.class_ = &JIVE_FLOAT_GATE;
	jive_float_gate_init_(gate, graph, name);
	return &gate->base.base;
}

jive_type *
jive_float_type_copy_(const jive_type * self_, jive_context * context)
{
	const jive_float_type * self = (const jive_float_type *) self_;
	jive_float_type * type = jive_context_malloc(context, sizeof(*type));
	*type = *self;
	return &type->base.base;
}

static inline void
jive_float_type_init_(jive_float_type * self)
{
	self->base.base.class_ = &JIVE_FLOAT_TYPE;
}

/* float_input inheritable members */

void
jive_float_input_init_(jive_float_input * self, struct jive_node * node, size_t index,
	jive_output * origin)
{
	jive_value_input_init_(&self->base, node, index, origin);
	jive_float_type_init_(&self->type);
}

const jive_type *
jive_float_input_get_type_(const jive_input * self_)
{
	const jive_float_input * self = (const jive_float_input *) self_;
	return &self->type.base.base;
}

/* float_output inheritable members */

void
jive_float_output_init_(jive_float_output * self, struct jive_node * node, size_t index)
{
	self->base.base.class_ = &JIVE_FLOAT_OUTPUT;
	jive_value_output_init_(&self->base, node, index);
	jive_float_type_init_(&self->type);
}

const jive_type *
jive_float_output_get_type_(const jive_output * self_)
{
	const jive_float_output * self = (const jive_float_output *) self_;
	return &self->type.base.base;
}

/* bitstring_gate inheritable members */

void
jive_float_gate_init_(jive_float_gate * self, struct jive_graph * graph, const char name[])
{
	self->base.base.class_ = &JIVE_FLOAT_GATE;
	jive_value_gate_init_(&self->base, graph, name);
	jive_float_type_init_(&self->type);
}

const jive_type *
jive_float_gate_get_type_(const jive_gate * self_)
{
	const jive_float_gate * self = (const jive_float_gate *) self_;
	return &self->type.base.base;
}
