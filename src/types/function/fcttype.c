/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/function/fcttype.h>

#include <string.h>
#include <stdio.h>

#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/valuetype-private.h>

/* function_type inheritable members */

static void
jive_function_type_fini_(jive_type * self);
static jive_input *
jive_function_type_create_input_(const jive_type * self, struct jive_node * node,
	size_t index, jive_output * initial_operand);
static jive_output *
jive_function_type_create_output_(const jive_type * self, struct jive_node * node,
	size_t index);
static jive_gate *
jive_function_type_create_gate_(const jive_type * self, struct jive_graph * graph, const char namae[]);
static bool
jive_function_type_equals_(const jive_type * self, const jive_type * other);
static jive_type *
jive_function_type_copy_(const jive_type * self, struct jive_context * context);

static void
jive_function_input_init_(jive_function_input * self, const jive_function_type * type,
	struct jive_node * node, size_t index, jive_output * origin);
static void
jive_function_input_fini_(jive_input * self);
static const jive_type *
jive_function_input_get_type_(const jive_input * self);

static void
jive_function_output_init_(jive_function_output * self, const jive_function_type * type,
	struct jive_node * node, size_t index);
static void
jive_function_output_fini_(jive_output * self);
static const jive_type *
jive_function_output_get_type_(const jive_output * self);


static void
jive_function_gate_init_(jive_function_gate * self, const jive_function_type * type,
	struct jive_graph * graph, const char name[]);
static void
jive_function_gate_fini_(jive_gate * self);
static const jive_type *
jive_function_gate_get_type_(const jive_gate * self); 


const jive_type_class JIVE_FUNCTION_TYPE = {
	parent : &JIVE_VALUE_TYPE,
	name : "fct",
	fini : jive_function_type_fini_, /* override */
	get_label : jive_type_get_label_, /* inherit */
	create_input : jive_function_type_create_input_, /* override */
	create_output : jive_function_type_create_output_, /* override */
	create_gate : jive_function_type_create_gate_, /* override */
	equals : jive_function_type_equals_, /* override */
	copy : jive_function_type_copy_, /* override */
};

const jive_input_class JIVE_FUNCTION_INPUT = {
	parent : &JIVE_VALUE_INPUT,
	fini : jive_function_input_fini_,  /* override */
	get_label : jive_input_get_label_, /* inherit */
	get_type : jive_function_input_get_type_, /* override */
};

const jive_output_class JIVE_FUNCTION_OUTPUT = {
	parent : &JIVE_VALUE_OUTPUT,
	fini : jive_function_output_fini_, /* override */
	get_label : jive_output_get_label_, /* inherit */
	get_type : jive_function_output_get_type_, /* override */
};

const jive_gate_class JIVE_FUNCTION_GATE = {
	parent : &JIVE_VALUE_GATE,
	fini : jive_function_gate_fini_, /* override */
	get_label : jive_gate_get_label_, /* inherit */
	get_type : jive_function_gate_get_type_, /* override */
};

void
jive_function_type_init(
	jive_function_type * self,
	jive_context * ctx,
	size_t narguments, const jive_type * const argument_types[],
	size_t nreturns, const jive_type * const return_types[])
{
	self->argument_types = jive_context_malloc(ctx, sizeof(jive_type *) * narguments); 
	self->return_types = jive_context_malloc(ctx, sizeof(jive_type *) * nreturns);
	
	size_t i;
	for(i = 0; i < narguments; i++)
		self->argument_types[i] = jive_type_copy(argument_types[i], ctx);
	
	for(i = 0; i < nreturns; i++)
		self->return_types[i] = jive_type_copy(return_types[i], ctx);
	
	self->base.base.class_ = &JIVE_FUNCTION_TYPE;
	self->ctx = ctx;
	self->nreturns = nreturns;
	self->narguments = narguments;
}

jive_function_type * jive_function_type_create(jive_context * ctx,
	size_t narguments, const jive_type * const argument_types[],
	size_t nreturns, const jive_type * const return_types[])
{
	jive_function_type * type = jive_context_malloc(ctx, sizeof(*type));
	jive_function_type_init(type, ctx, narguments, argument_types, nreturns, return_types);
	return type;
}

void jive_function_type_destroy(jive_function_type * type)
{
	jive_function_type_fini_(&type->base.base);
	jive_context_free(type->ctx, type);
}

/* function_type inheritable members */

void
jive_function_type_fini(jive_function_type * self)
{
	size_t i;
	for(i = 0; i < self->narguments; i++){
		jive_type_fini(self->argument_types[i]);
		jive_context_free(self->ctx, self->argument_types[i]);
	}
	jive_context_free(self->ctx, self->argument_types);
	
	for(i = 0; i < self->nreturns; i++){
		jive_type_fini(self->return_types[i]);
		jive_context_free(self->ctx, self->return_types[i]);
	}
	jive_context_free(self->ctx, self->return_types);
	
	jive_value_type_fini_(&self->base.base);
}

void
jive_function_type_fini_(jive_type * self_)
{
	jive_function_type * self = (jive_function_type *) self_; 
	jive_function_type_fini(self);
}


jive_type *
jive_function_type_copy_(const jive_type * self_, jive_context * ctx)
{
	const jive_function_type * self = (const jive_function_type *) self_;
	
	jive_function_type * type = jive_context_malloc(ctx, sizeof( *type));
	
	jive_function_type_init(type, ctx,
		self->narguments, (const jive_type * const *) self->argument_types,
		self->nreturns, (const jive_type * const *) self->return_types);
	
	return &type->base.base;
}

jive_input *
jive_function_type_create_input_(const jive_type * self_, struct jive_node * node, size_t index, jive_output * initial_operand)
{
	const jive_function_type * self = (const jive_function_type *) self_;
	jive_function_input * input = jive_context_malloc(node->graph->context, sizeof( *input));
	
	input->base.base.class_ = &JIVE_FUNCTION_INPUT;
	jive_function_input_init_(input, self, node, index, initial_operand);
	
	return &input->base.base;
}

jive_output *
jive_function_type_create_output_(const jive_type * self_, struct jive_node * node, size_t index)
{
	const jive_function_type * self = (const jive_function_type *) self_;
	jive_function_output * output = jive_context_malloc(node->graph->context, sizeof( *output));
	
	output->base.base.class_ = &JIVE_FUNCTION_OUTPUT;
	jive_function_output_init_(output, self, node, index);
	
	return &output->base.base;
}

bool
jive_function_type_equals_(const jive_type * self_, const jive_type * other_)
{
	const jive_function_type * self = (const jive_function_type *) self_;
	const jive_function_type * other = (const jive_function_type *) other_;
	
	if (self->nreturns != other->nreturns) return false;
	if (self->narguments != other->narguments) return false;
	
	size_t i;
	for(i = 0; i < self->nreturns; i++){
		if (!jive_type_equals(self->return_types[i], other->return_types[i]))
			return false;
	}
	
	for(i = 0; i < self->narguments; i++){
		if (!jive_type_equals(self->argument_types[i], other->argument_types[i]))
			return false;
	}
	
	return true; 
}

jive_gate *
jive_function_type_create_gate_(const jive_type * self_, struct jive_graph * graph, const char * name)
{
	const jive_function_type * self = (const jive_function_type *) self_;
	
	jive_function_gate * gate = jive_context_malloc(graph->context, sizeof( *gate));
	
	gate->base.base.class_ = &JIVE_FUNCTION_GATE;
	jive_function_gate_init_(gate, self, graph, name);
	
	return &gate->base.base;
}

/* function_input inheritable members */

void
jive_function_input_init_(jive_function_input * self, const jive_function_type * type,
	struct jive_node * node, size_t index, jive_output * origin)
{
	jive_value_input_init_(&self->base, node, index, origin);
	
	jive_function_type_init(&self->type, 
		node->graph->context,
		type->narguments, (const jive_type * const *)type->argument_types,
		type->nreturns, (const jive_type * const *)type->return_types);
}

void
jive_function_input_fini_(jive_input * self_)
{
	jive_function_input * self = (jive_function_input *) self_;
	
	jive_function_type_fini(&self->type);
	
	jive_input_fini_(&self->base.base);
}

const jive_type *
jive_function_input_get_type_(const jive_input * self_)
{
	const jive_function_input * self = (const jive_function_input *) self_;
	
	return &self->type.base.base;
}

/* function_output inheritable members */

void
jive_function_output_init_(jive_function_output * self, const jive_function_type * type,
  struct jive_node * node, size_t index)
{
	jive_value_output_init_(&self->base, node, index);
	
	jive_function_type_init(&self->type, 
		node->graph->context,
		type->narguments, (const jive_type * const *)type->argument_types,
		type->nreturns, (const jive_type * const *)type->return_types);
} 

void
jive_function_output_fini_(jive_output * self_)
{
	jive_function_output * self = (jive_function_output *) self_;
	
	jive_function_type_fini(&self->type);
	
	jive_output_fini_(&self->base.base);
}

const jive_type *
jive_function_output_get_type_(const jive_output * self_)
{
	const jive_function_output * self = (const jive_function_output *) self_;
	
	return &self->type.base.base;
}

/* function_gate inheritable members */

void
jive_function_gate_init_(jive_function_gate * self, const jive_function_type * type,
  struct jive_graph * graph,  const char name[])
{
	jive_value_gate_init_(&self->base, graph, name);
	jive_function_type_init(&self->type, 
		graph->context,
		type->narguments, (const jive_type * const *)type->argument_types,
		type->nreturns, (const jive_type * const *)type->return_types);
}

void
jive_function_gate_fini_(jive_gate * self_)
{
	jive_function_gate * self = (jive_function_gate *) self_;
	
	jive_function_type_fini(&self->type);
	
	jive_gate_fini_(&self->base.base);
}

const jive_type *
jive_function_gate_get_type_(const jive_gate * self_)
{
	const jive_function_gate * self = (const jive_function_gate *) self_;
	
	return &self->type.base.base;
}
