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
jive_integral_type_copy_(const jive_type * self_, struct jive_context * context);

static jive_input *
jive_integral_type_create_input_(const jive_type * self_, struct jive_node * node, size_t index,
	jive_output * origin);

static jive_output *
jive_integral_type_create_output_(const jive_type * self_, struct jive_node * node, size_t index);

static jive_gate *
jive_integral_type_create_gate_(const jive_type * self_, struct jive_graph * graph,
	const char * name);

static inline void
jive_integral_input_init_(jive_integral_input * self_, struct jive_node * node, size_t index,
	jive_output * origin);

static const jive_type *
jive_integral_input_get_type_(const jive_input * self_);

static inline void
jive_integral_output_init_(jive_integral_output * self_, struct jive_node * node, size_t index);

static const jive_type *
jive_integral_output_get_type_(const jive_output * self_);

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

const jive_input_class JIVE_INTEGRAL_INPUT = {
	parent : &JIVE_VALUE_INPUT,
	fini : jive_input_fini_, /* inherit */
	get_label : jive_input_get_label_, /* inherit */
	get_type : jive_integral_input_get_type_, /* override */
};

const jive_output_class JIVE_INTEGRAL_OUTPUT = {
	parent : &JIVE_VALUE_OUTPUT,
	fini : jive_output_fini_, /* inherit */
	get_label : jive_output_get_label_, /* inherit */
	get_type : jive_integral_output_get_type_, /* override */
};

const jive_gate_class JIVE_INTEGRAL_GATE = {
	parent : &JIVE_VALUE_GATE,
	fini : jive_gate_fini_, /* inherit */
	get_label : jive_gate_get_label_, /* inherit */
	get_type : jive_integral_gate_get_type_, /* override */
};

/* integral_type members */

static inline void
jive_integral_type_init_(jive_integral_type * self)
{
	self->class_ = &JIVE_INTEGRAL_TYPE;
}

static void
jive_integral_type_fini_(jive_type * self_)
{
	jive_integral_type * self = (jive_integral_type *) self_;
	jive_value_type_fini_(self);
}

static jive_type *
jive_integral_type_copy_(const jive_type * self_, struct jive_context * context)
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
	jive_integral_input * input = jive_context_malloc(node->graph->context, sizeof(*input));
	input->base.base.class_ = &JIVE_INTEGRAL_INPUT;
	jive_integral_input_init_(input, node, index, origin);

	return &input->base.base;
}

static jive_output *
jive_integral_type_create_output_(const jive_type * self_, struct jive_node * node, size_t index)
{
	jive_integral_output * output = jive_context_malloc(node->graph->context, sizeof(*output));
	output->base.base.class_ = &JIVE_INTEGRAL_OUTPUT;
	jive_integral_output_init_(output, node, index);

	return &output->base.base;
}

static jive_gate *
jive_integral_type_create_gate_(const jive_type * self_, struct jive_graph * graph,
	const char * name)
{
	jive_integral_gate * gate = jive_context_malloc(graph->context, sizeof(*gate));
	gate->base.base.class_ = &JIVE_INTEGRAL_GATE;
	jive_integral_gate_init_(gate, graph, name);

	return &gate->base.base;
}

/* integral_input members */

static inline void
jive_integral_input_init_(jive_integral_input * self, struct jive_node * node, size_t index,
	jive_output * origin)
{
	jive_value_input_init_(&self->base, node, index, origin);
	jive_integral_type_init_(&self->type);
}

static const jive_type *
jive_integral_input_get_type_(const jive_input * self_)
{
	const jive_integral_input * self = (const jive_integral_input *) self_;

	return &self->type;
}

/* integral_output members */

static inline void
jive_integral_output_init_(jive_integral_output * self, struct jive_node * node, size_t index)
{
	self->base.base.class_ = &JIVE_INTEGRAL_OUTPUT;
	jive_value_output_init_(&self->base, node, index);
	jive_integral_type_init_(&self->type);
}

static const jive_type *
jive_integral_output_get_type_(const jive_output * self_)
{
	const jive_integral_output * self = (const jive_integral_output *) self_;

	return &self->type;
}

/* integral_gate members */

static inline void
jive_integral_gate_init_(jive_integral_gate * self, struct jive_graph * graph,
	const char name[])
{
	self->base.base.class_ = &JIVE_INTEGRAL_GATE;
	jive_value_gate_init_(&self->base, graph, name);
	jive_integral_type_init_(&self->type);
}

static const jive_type *
jive_integral_gate_get_type_(const jive_gate * self_)
{
	const jive_integral_gate * self = (const jive_integral_gate *) self_;

	return &self->type;
}
