/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
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
jive_address_type_copy_(const jive_type * self, jive_context * context);

static void
jive_address_input_init_(jive_address_input * self, const jive_address_type * type,
	jive_node * node, size_t index, jive_output * origin);
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
	.parent = &JIVE_VALUE_TYPE,
	.name = "addr",
	.fini = jive_value_type_fini_, /* inherit */
	.get_label = jive_type_get_label_, /* inherit */
	.create_input = jive_address_type_create_input_, /* override */
	.create_output = jive_address_type_create_output_, /* override */
	.create_gate = jive_address_type_create_gate_, /* override */
	.equals = jive_type_equals_, /* inherit */
	.copy = jive_address_type_copy_, /* override */
};

const jive_input_class JIVE_ADDRESS_INPUT = {
	.parent = &JIVE_VALUE_INPUT,
	.fini = jive_address_input_fini_, /* override */
	.get_label = jive_input_get_label_, /* inherit */
	.get_type = jive_address_input_get_type_ /* override */
};

const jive_output_class JIVE_ADDRESS_OUTPUT = {
	.parent = &JIVE_VALUE_OUTPUT,
	.fini = jive_address_output_fini_, /* override */
	.get_label = jive_output_get_label_, /* inherit */
	.get_type = jive_address_output_get_type_ /* override */
};

const jive_gate_class JIVE_ADDRESS_GATE = {
	.parent = &JIVE_VALUE_GATE,
	.fini = jive_address_gate_fini_, /* inherit */
	.get_label = jive_gate_get_label_, /* inherit */
	.get_type = jive_address_gate_get_type_ /* override */
};

void
jive_address_type_init(jive_address_type * self)
{
	self->base.base.class_ = &JIVE_ADDRESS_TYPE;
}

/* address_type inheritable members */

jive_type *
jive_address_type_copy_(const jive_type * self_, jive_context * context)
{
	jive_address_type * type = jive_context_malloc(context, sizeof(*type));

	jive_address_type_init(type);

	return &type->base.base;	
}

jive_input *
jive_address_type_create_input_(const jive_type * self_, jive_node * node,
	size_t index, jive_output * initial_operand)
{
	const jive_address_type * self = (const jive_address_type *) self_;
	jive_address_input * input = jive_context_malloc(node->graph->context, sizeof(*input));

	input->base.base.class_ = &JIVE_ADDRESS_INPUT;
	jive_address_input_init_(input, self, node, index, initial_operand);

	return &input->base.base;
}

jive_output *
jive_address_type_create_output_(const jive_type * self_, jive_node * node, size_t index)
{
	const jive_address_type * self = (const jive_address_type *) self_;
	jive_address_output * output = jive_context_malloc(node->graph->context, sizeof(*output));

	output->base.base.class_ = &JIVE_ADDRESS_OUTPUT;
	jive_address_output_init_(output, self, node, index);

	return &output->base.base;
}

jive_gate *
jive_address_type_create_gate_(const jive_type * self_, jive_graph * graph,
	const char * name)
{
	const jive_address_type * self = (const jive_address_type *)self_;

	jive_address_gate * gate = jive_context_malloc(graph->context, sizeof(*gate));

	gate->base.base.class_ = &JIVE_ADDRESS_GATE;
	jive_address_gate_init_(gate, self, graph, name);

	return &gate->base.base;
}

/* address_input inheritable members */

void
jive_address_input_init_(jive_address_input * self, const jive_address_type * type,
	jive_node * node, size_t index, jive_output * origin)
{
	jive_value_input_init_(&self->base, node, index, origin);
	jive_address_type_init(&self->type);
}

void
jive_address_input_fini_(jive_input * self_)
{
	jive_address_input * self = (jive_address_input *) self_;

	self->type.base.base.class_->fini(&self->type.base.base);
	jive_input_fini_(&self->base.base);
}

const jive_type *
jive_address_input_get_type_(const jive_input * self_)
{
	const jive_address_input * self = (const jive_address_input *)self_;
	return &self->type.base.base;
}

/* address_output inheritable members */

void
jive_address_output_init_(jive_address_output * self, const jive_address_type * type,
	jive_node * node, size_t index)
{
	jive_value_output_init_(&self->base, node, index);
	jive_address_type_init(&self->type);
}

void
jive_address_output_fini_(jive_output * self_)
{
	jive_address_output * self = (jive_address_output *)self_;

	self->type.base.base.class_->fini(&self->type.base.base);
	jive_output_fini_(&self->base.base);
}

const jive_type *
jive_address_output_get_type_(const jive_output * self_)
{
	const jive_address_output * self = (const jive_address_output *)self_;

	return &self->type.base.base;
}

/*address_gate inheritable members */

void
jive_address_gate_init_(jive_address_gate * self, const jive_address_type * type,
	jive_graph * graph, const char name[])
{
	jive_value_gate_init_(&self->base, graph, name);
	jive_address_type_init(&self->type);
}

void jive_address_gate_fini_(jive_gate * self_)
{
	jive_address_gate * self = (jive_address_gate *)self_;

	self->type.base.base.class_->fini(&self->type.base.base);
	jive_gate_fini_(&self->base.base);	
}

const jive_type *
jive_address_gate_get_type_(const jive_gate * self_)
{
	const jive_address_gate * self = (const jive_address_gate *) self_;

	return &self->type.base.base;
}
