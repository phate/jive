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

static const jive_value_type jive_value_type_singleton = {
	base : {class_ : &JIVE_VALUE_TYPE}
};

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

const jive_input_class JIVE_VALUE_INPUT = {
	parent : &JIVE_INPUT,
	fini : jive_input_fini_, /* inherit */
	get_label : jive_input_get_label_, /* inherit */
	get_type : jive_value_input_get_type_, /* override */
};

const jive_output_class JIVE_VALUE_OUTPUT = {
	parent : &JIVE_OUTPUT,
	fini : jive_output_fini_, /* inherit */
	get_label : jive_output_get_label_, /* inherit */
	get_type : jive_value_output_get_type_, /* override */
};

const jive_gate_class JIVE_VALUE_GATE = {
	parent : &JIVE_GATE,
	fini : jive_gate_fini_, /* inherit */
	get_label : jive_gate_get_label_, /* inherit */
	get_type : jive_value_gate_get_type_, /* override */
};

void
jive_value_type_fini_(jive_type * self_)
{
	jive_value_type * self = (jive_value_type *) self_;
	jive_type_fini_( &self->base ) ;
}

jive_type *
jive_value_type_copy_(const jive_type * self_, jive_context * context)
{
	const jive_value_type * self = (const jive_value_type *) self_;
	
	jive_value_type * type = new jive_value_type;
	
	type->base = self->base;
	
	return &type->base;
} 

jive_input *
jive_value_type_create_input_(const jive_type * self, struct jive_node * node, size_t index, jive_output * initial_operand)
{
	jive_value_input * input = jive_context_malloc(node->graph->context, sizeof(*input));
	input->base.class_ = &JIVE_VALUE_INPUT;
	jive_value_input_init_(input, node, index, initial_operand);
	return &input->base; 
}

jive_output *
jive_value_type_create_output_(const jive_type * self, struct jive_node * node, size_t index)
{
	jive_value_output * output = jive_context_malloc(node->graph->context, sizeof(*output));
	output->base.class_ = &JIVE_VALUE_OUTPUT;
	jive_value_output_init_(output, node, index);
	return &output->base; 
}

jive_gate *
jive_value_type_create_gate_(const jive_type * self, struct jive_graph * graph, const char * name)
{
	jive_value_gate * gate = jive_context_malloc(graph->context, sizeof(*gate));
	gate->base.class_ = &JIVE_VALUE_GATE;
	jive_value_gate_init_(gate, graph, name);
	return &gate->base; 
}

/* value inputs */

void
jive_value_input_init_(jive_value_input * self, struct jive_node * node, size_t index, jive_output * origin)
{
	jive_input_init_(&self->base, node, index, origin);
}

const jive_type *
jive_value_input_get_type_(const jive_input * self)
{
	return &jive_value_type_singleton.base;
}

/* value outputs */

void
jive_value_output_init_(jive_value_output * self, struct jive_node * node, size_t index)
{
	jive_output_init_(&self->base, node, index);
}

const jive_type *
jive_value_output_get_type_(const jive_output * self)
{
	return &jive_value_type_singleton.base;
}

/* value gates */

void
jive_value_gate_init_(jive_value_gate * self, struct jive_graph * graph, const char * name)
{
	jive_gate_init_(&self->base, graph, name);
}

const jive_type *
jive_value_gate_get_type_(const jive_gate * self)
{
	return &jive_value_type_singleton.base;
}
