/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/statetype.h>
#include <jive/vsdg/statetype-private.h>

#include <jive/common.h>
#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

const jive_type_class JIVE_STATE_TYPE = {
	parent : &JIVE_TYPE,
	name : "X",
	fini : jive_state_type_fini_, /* override */
	get_label : jive_type_get_label_, /* inherit */
	create_input : jive_state_type_create_input_, /* override */
	create_output : jive_state_type_create_output_, /* override */
	create_gate : jive_state_type_create_gate_, /* override */
	equals : jive_type_equals_, /* inherit */
	copy : jive_state_type_copy_, /* override */
};

const jive_input_class JIVE_STATE_INPUT = {
	parent : &JIVE_INPUT,
	fini : jive_input_fini_, /* inherit */
	get_label : jive_input_get_label_, /* inherit */
	get_type : jive_state_input_get_type_, /* override */
};

const jive_output_class JIVE_STATE_OUTPUT = {
	parent : &JIVE_OUTPUT,
	fini : jive_output_fini_, /* inherit */
	get_label : jive_output_get_label_, /* inherit */
	get_type : jive_state_output_get_type_, /* override */
};

const jive_gate_class JIVE_STATE_GATE = {
	parent : &JIVE_GATE,
	fini : jive_gate_fini_, /* inherit */
	get_label : jive_gate_get_label_, /* inherit */
	get_type : jive_state_gate_get_type_, /* override */
};

void
jive_state_type_fini_(jive_type * self)
{
	jive_type_fini_(self);
}

jive_type *
jive_state_type_copy_(const jive_type * self_)
{
	const jive_state_type * self = (const jive_state_type *) self_;
	
	jive_state_type * type = new jive_state_type;
	type->class_ = &JIVE_STATE_TYPE;	
	
	return type;
}

jive_input *
jive_state_type_create_input_(const jive_type * self, struct jive_node * node, size_t index, jive_output * initial_operand)
{
	jive_state_input * input = jive_context_malloc(node->graph->context, sizeof(*input));
	input->class_ = &JIVE_STATE_INPUT;
	jive_state_input_init_(input, node, index, initial_operand);
	return input;
}

jive_output *
jive_state_type_create_output_(const jive_type * self, struct jive_node * node, size_t index)
{
	jive_state_output * output = jive_context_malloc(node->graph->context, sizeof(*output));
	output->class_ = &JIVE_STATE_OUTPUT;
	jive_state_output_init_(output, node, index);
	return output;
}

jive_gate *
jive_state_type_create_gate_(const jive_type * self, struct jive_graph * graph, const char * name)
{
	jive_state_gate * gate = jive_context_malloc(graph->context, sizeof(*gate));
	gate->class_ = &JIVE_STATE_GATE;
	jive_state_gate_init_(gate, graph, name);
	return gate;
}

void
jive_state_input_init_(jive_state_input * self, struct jive_node * node, size_t index, jive_output * origin)
{
	jive_input_init_(self, node, index, origin);
}

const jive_type *
jive_state_input_get_type_(const jive_input * self)
{
	static jive_state_type state_type; state_type.class_ = &JIVE_STATE_TYPE;
	return &state_type;
}

void
jive_state_output_init_(jive_state_output * self, struct jive_node * node, size_t index)
{
	jive_output_init_(self, node, index);
}

const jive_type *
jive_state_output_get_type_(const jive_output * self)
{
	static jive_state_type state_type; state_type.class_ = &JIVE_STATE_TYPE;
	return &state_type;
}

void
jive_state_gate_init_(jive_state_gate * self, struct jive_graph * graph, const char * name)
{
	jive_gate_init_(self, graph, name);
}

const jive_type *
jive_state_gate_get_type_(const jive_gate * self)
{
	static jive_state_type state_type; state_type.class_ = &JIVE_STATE_TYPE;
	return &state_type;
}














static void
jive_statemux_node_fini_(jive_node * self_);

static const jive_node_attrs *
jive_statemux_node_get_attrs_(const jive_node * self_);

static bool
jive_statemux_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_);

static jive_node *
jive_statemux_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[]);

const jive_node_class JIVE_STATEMUX_NODE = {
	parent : &JIVE_NODE,
	name : "STATEMUX",
	fini : jive_statemux_node_fini_, /* override */
	get_default_normal_form : jive_node_get_default_normal_form_, /* inherit */
	get_label : jive_node_get_label_, /* inherit */
	get_attrs : jive_statemux_node_get_attrs_, /* override */
	match_attrs : jive_statemux_node_match_attrs_, /* override */
	check_operands : NULL,
	create : jive_statemux_node_create_, /* override */
};

static void
jive_statemux_node_fini_(jive_node * self_)
{
	jive_context * context = self_->graph->context;
	jive_statemux_node * self = (jive_statemux_node *) self_;

	jive_type_destroy(self->attrs.type);
	
	jive_node_fini_(self);
}

static const jive_node_attrs *
jive_statemux_node_get_attrs_(const jive_node * self_)
{
	const jive_statemux_node * self = (const jive_statemux_node *) self_;
	return &self->attrs;
}

static bool
jive_statemux_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_)
{
	const jive_statemux_node * self = (const jive_statemux_node *) self_;
	const jive_statemux_node_attrs * attrs = (const jive_statemux_node_attrs *) attrs_;
	return jive_type_equals(self->attrs.type, attrs->type);
}

static jive_node *
jive_statemux_node_create_(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[])
{
	const jive_statemux_node_attrs * attrs = (const jive_statemux_node_attrs *) attrs_;
	return jive_statemux_node_create(region, attrs->type, noperands, operands, attrs->noutputs);
}

jive_node *
jive_statemux_node_create(jive_region * region,
	const jive_type * statetype,
	size_t noperands, jive_output * const operands[],
	size_t noutputs)
{
	jive_context * context = region->graph->context;
	jive_statemux_node * node = new jive_statemux_node;
	
	node->class_ = &JIVE_STATEMUX_NODE;
	JIVE_DEBUG_ASSERT(jive_type_isinstance(statetype, &JIVE_STATE_TYPE));
	
	const jive_type * operand_types[noperands];
	const jive_type * output_types[noutputs];
	size_t n;
	for (n = 0; n < noperands; n++)
		operand_types[n] = statetype;
	for (n = 0; n < noutputs; n++)
		output_types[n] = statetype;
		
	jive_node_init_(node, region,
		noperands, operand_types, operands,
		noutputs, output_types);
	node->attrs.type = jive_type_copy(statetype);
	node->attrs.noutputs = noutputs;
	
	return node;
}

jive_output *
jive_state_merge(const jive_type * statetype, size_t nstates, jive_output * const states[])
{
	jive_region * region = jive_region_innermost(nstates, states);
	return jive_statemux_node_create(region, statetype, nstates, states, 1)->outputs[0];
}

jive_node *
jive_state_split(const jive_type * statetype, jive_output * state, size_t nstates)
{
	jive_region * region = state->node->region;
	return jive_statemux_node_create(region, statetype, 1, &state, nstates);
}
