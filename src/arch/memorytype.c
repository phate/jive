/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/memorytype.h>

#include <string.h>

#include <jive/context.h>
#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/statetype-private.h>

static void
jive_memory_input_init_(jive_memory_input * self, struct jive_node * node, size_t index, jive_output * origin)
{
	jive_state_input_init_(self, node, index, origin);
}

static const jive_type *
jive_memory_input_get_type_(const jive_input * self)
{
	static jive_memory_type memory_type; memory_type.class_ = &JIVE_MEMORY_TYPE;
	return &memory_type;
}

static void
jive_memory_output_init_(jive_memory_output * self, struct jive_node * node, size_t index)
{
	jive_state_output_init_(self, node, index);
}

static const jive_type *
jive_memory_output_get_type_(const jive_output * self)
{
	static jive_memory_type memory_type; memory_type.class_ = &JIVE_MEMORY_TYPE;
	return &memory_type;
}

static void
jive_memory_gate_init_(jive_memory_gate * self, struct jive_graph * graph, const char * name)
{
	jive_state_gate_init_(&self->base, graph, name);
}

static const jive_type *
jive_memory_gate_get_type_(const jive_gate * self)
{
	static jive_memory_type memory_type; memory_type.class_ = &JIVE_MEMORY_TYPE;
	return &memory_type;
}

static jive_type *
jive_memory_type_copy_(const jive_type * self_, jive_context * context)
{
	const jive_memory_type * self = (const jive_memory_type *) self_;
	
	jive_memory_type * type = new jive_memory_type;
	type->class_ = &JIVE_MEMORY_TYPE;	
	
	return type;
}

static jive_input *
jive_memory_type_create_input_(const jive_type * self, struct jive_node * node, size_t index, jive_output * initial_operand)
{
	jive_memory_input * input = jive_context_malloc(node->graph->context, sizeof(*input));
	input->class_ = &JIVE_MEMORY_INPUT;
	jive_memory_input_init_(input, node, index, initial_operand);
	return input;
}

static jive_output *
jive_memory_type_create_output_(const jive_type * self, struct jive_node * node, size_t index)
{
	jive_memory_output * output = jive_context_malloc(node->graph->context, sizeof(*output));
	output->class_ = &JIVE_MEMORY_OUTPUT;
	jive_memory_output_init_(output, node, index);
	return output;
}

static jive_gate *
jive_memory_type_create_gate_(const jive_type * self, struct jive_graph * graph, const char * name)
{
	jive_memory_gate * gate = jive_context_malloc(graph->context, sizeof(*gate));
	gate->base.base.class_ = &JIVE_MEMORY_GATE;
	jive_memory_gate_init_(gate, graph, name);
	return &gate->base.base;
}

const jive_type_class JIVE_MEMORY_TYPE = {
	parent : &JIVE_STATE_TYPE,
	name : "mem",
	fini : jive_state_type_fini_, /* inherit */
	get_label : jive_type_get_label_, /* inherit */
	create_input : jive_memory_type_create_input_, /* override */
	create_output : jive_memory_type_create_output_, /* override */
	create_gate : jive_memory_type_create_gate_, /* override */
	equals : jive_type_equals_, /* inherit */
	copy : jive_memory_type_copy_, /* override */
};

const jive_input_class JIVE_MEMORY_INPUT = {
	parent : &JIVE_STATE_INPUT,
	fini : jive_input_fini_, /* inherit */
	get_label : jive_input_get_label_, /* inherit */
	get_type : jive_memory_input_get_type_, /* override */
};

const jive_output_class JIVE_MEMORY_OUTPUT = {
	parent : &JIVE_STATE_OUTPUT,
	fini : jive_output_fini_, /* inherit */
	get_label : jive_output_get_label_, /* inherit */
	get_type : jive_memory_output_get_type_, /* override */
};

const jive_gate_class JIVE_MEMORY_GATE = {
	parent : &JIVE_STATE_GATE,
	fini : jive_gate_fini_, /* inherit */
	get_label : jive_gate_get_label_, /* inherit */
	get_type : jive_memory_gate_get_type_, /* override */
};

