/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/function/fctlambda.h>

#include <stdio.h>
#include <string.h>

#include <jive/vsdg/controltype.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/substitution.h>

/* lambda enter node */

static jive_node *
jive_lambda_enter_node_create(jive_region * region)
{
	JIVE_DEBUG_ASSERT(region->top == NULL && region->bottom == NULL);
	jive_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	
	node->class_ = &JIVE_LAMBDA_ENTER_NODE;
	JIVE_DECLARE_CONTROL_TYPE(ctl);
	jive_node_init_(node, region,
		0, NULL, NULL,
		1, &ctl);
	((jive_control_output *)node->outputs[0])->active = false;
	region->top = node;
	
	return node;
}

static jive_node *
jive_lambda_enter_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[])
{
	return jive_lambda_enter_node_create(region);
}

const jive_node_class JIVE_LAMBDA_ENTER_NODE = {
	.parent = &JIVE_NODE,
	.name = "LAMBDA_ENTER",
	.fini = jive_node_fini_, /* inherit */
	.get_default_normal_form = jive_node_get_default_normal_form_, /* inherit */
	.get_label = jive_node_get_label_, /* inherit */
	.get_attrs = jive_node_get_attrs_, /* inherit */
	.match_attrs = jive_node_match_attrs_, /* inherit */
	.check_operands = NULL,
	.create = jive_lambda_enter_node_create_, /* override */
	.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
};

/* lambda leave node */

static jive_node *
jive_lambda_leave_node_create(jive_output * output)
{
	JIVE_DEBUG_ASSERT(output->node->region->bottom == NULL);
	
	jive_node * node = jive_context_malloc(output->node->graph->context, sizeof(*node));
	
	node->class_ = &JIVE_LAMBDA_LEAVE_NODE;
	JIVE_DECLARE_CONTROL_TYPE(ctl);
	JIVE_DECLARE_ANCHOR_TYPE(anchor);
	jive_node_init_(node, output->node->region,
		1, &ctl, &output,
		1, &anchor);
	output->node->region->bottom = node;
	
	return node;
}

static jive_node *
jive_lambda_leave_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands > 0);
	return jive_lambda_leave_node_create(operands[0]);
}

const jive_node_class JIVE_LAMBDA_LEAVE_NODE = {
	.parent = &JIVE_NODE,
	.name = "LAMBDA_LEAVE",
	.fini = jive_node_fini_, /* inherit */
	.get_default_normal_form = jive_node_get_default_normal_form_, /* inherit */
	.get_label = jive_node_get_label_, /* inherit */
	.get_attrs = jive_node_get_attrs_, /* inherit */
	.match_attrs = jive_node_match_attrs_, /* inherit */
	.check_operands = NULL,
	.create = jive_lambda_leave_node_create_, /* override */
	.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
};

/* lambda node */

static void
jive_lambda_node_init_(jive_lambda_node * self, jive_region * function_region)
{
	jive_region * region = function_region->parent;
	jive_context * context = function_region->graph->context;
	
	size_t narguments = function_region->top->noutputs - 1;
	size_t nreturns = function_region->bottom->ninputs - 1;
	size_t n;
	
	const jive_type * argument_types[narguments];
	for(n = 0; n < narguments; n++)
		argument_types[n] = jive_output_get_type(function_region->top->outputs[n+1]);
	const jive_type * return_types[nreturns];
	for(n = 0; n < nreturns; n++)
		return_types[n] = jive_input_get_type(function_region->bottom->inputs[n+1]);
	
	jive_function_type_init(&self->attrs.function_type, context,
		narguments, argument_types,
		nreturns, return_types);
	
	JIVE_DECLARE_ANCHOR_TYPE(anchor_type);
	
	const jive_type * function_type = &self->attrs.function_type.base.base;
	jive_node_init_(&self->base, region,
		1, &anchor_type, &function_region->bottom->outputs[0],
		1, &function_type);
	
	self->attrs.argument_gates = jive_context_malloc(context, narguments * sizeof(jive_gate *));
	self->attrs.return_gates = jive_context_malloc(context, nreturns * sizeof(jive_gate *));
	for(n = 0; n < narguments; n++)
		self->attrs.argument_gates[n] = function_region->top->outputs[n+1]->gate;
	for(n = 0; n < nreturns; n++)
		self->attrs.return_gates[n] = function_region->bottom->inputs[n+1]->gate;
}

static jive_node *
jive_lambda_node_create(jive_region * function_region)
{
	jive_lambda_node * node = jive_context_malloc(function_region->graph->context, sizeof(*node));
	node->base.class_ = &JIVE_LAMBDA_NODE;
	jive_lambda_node_init_(node, function_region);
	return &node->base;
}

static void
jive_lambda_node_fini_(jive_node * self_)
{
	jive_context * context = self_->graph->context;
	jive_lambda_node * self = (jive_lambda_node *) self_;
	
	jive_function_type_fini(&self->attrs.function_type);
	jive_context_free(context, self->attrs.argument_gates);
	jive_context_free(context, self->attrs.return_gates);
	
	jive_node_fini_(&self->base);
}

static jive_node *
jive_lambda_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands > 0);
	return jive_lambda_node_create(operands[0]->node->region);
}

static const jive_node_attrs *
jive_lambda_node_get_attrs_(const jive_node * self_)
{
	const jive_lambda_node * self = (const jive_lambda_node *) self_;
	
	return &self->attrs.base;
}

static bool
jive_lambda_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive_lambda_node_attrs * first = &((const jive_lambda_node *)self)->attrs;
	const jive_lambda_node_attrs * second = (const jive_lambda_node_attrs *) attrs;

	if (!jive_type_equals(&first->function_type.base.base, &second->function_type.base.base)) return false;
	size_t n;
	for(n = 0; n < first->function_type.narguments; n++)
		if (first->argument_gates[n] != second->argument_gates[n]) return false;
	for(n = 0; n < first->function_type.nreturns; n++)
		if (first->return_gates[n] != second->return_gates[n]) return false;

	return true;
}

const jive_node_class JIVE_LAMBDA_NODE = {
	.parent = &JIVE_NODE,
	.name = "LAMBDA",
	.fini = jive_lambda_node_fini_, /* override */
	.get_default_normal_form = jive_node_get_default_normal_form_, /* inherit */
	.get_label = jive_node_get_label_, /* inherit */
	.get_attrs = jive_lambda_node_get_attrs_, /* inherit */
	.match_attrs = jive_lambda_node_match_attrs_, /* override */
	.check_operands = NULL,
	.create = jive_lambda_node_create_, /* override */
	.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
};

/* lambda instantiation */

typedef struct jive_lambda_build_state jive_lambda_build_state;
struct jive_lambda_build_state {
	jive_floating_region floating;
};

struct jive_lambda *
jive_lambda_begin(struct jive_graph * graph,
	size_t narguments, const jive_type * const argument_types[], const char * const argument_names[])
{
	jive_context * context = graph->context;

	jive_lambda * lambda = jive_context_malloc(context, sizeof(*lambda));
	jive_lambda_build_state * state;
	state = jive_context_malloc(graph->context, sizeof(*state));
	state->floating = jive_floating_region_create(graph);
	lambda->region = state->floating.region;
	lambda->arguments = jive_context_malloc(graph->context, sizeof(*lambda->arguments) * narguments);
	lambda->narguments = narguments;

	jive_lambda_enter_node_create(lambda->region);

	size_t n;
	for (n = 0; n < narguments; n++) {
		jive_gate * gate = jive_type_create_gate(argument_types[n], graph, argument_names[n]);
		lambda->arguments[n] = jive_node_gate_output(lambda->region->top, gate);
	}

	lambda->internal_state = state;

	return lambda;
}

jive_output *
jive_lambda_end(jive_lambda * self,
	size_t nresults, const jive_type * const result_types[], jive_output * const results[])
{
	jive_lambda_build_state * state = self->internal_state;
	jive_region * region = self->region;
	jive_graph * graph = region->graph;
	jive_context * context = graph->context;

	jive_node * leave = jive_lambda_leave_node_create(region->top->outputs[0]);

	size_t n;
	for (n = 0; n < nresults; n++) {
		char gate_name[80];
		snprintf(gate_name, sizeof(gate_name), "res_%p_%zd", leave, n);
		jive_gate * gate = jive_type_create_gate(result_types[n], graph, gate_name);
		jive_node_gate_input(leave, gate, results[n]);
	}

	jive_floating_region_settle(state->floating);

	jive_node * anchor = jive_lambda_node_create(region);
	JIVE_DEBUG_ASSERT(anchor->noutputs == 1);

	jive_context_free(context, self->arguments);
	jive_context_free(context, state);
	jive_context_free(context, self);

	return anchor->outputs[0];
}

void
jive_inline_lambda_apply(jive_node * apply_node)
{
	jive_lambda_node * lambda_node = jive_lambda_node_cast(apply_node->inputs[0]->origin->node);
	JIVE_DEBUG_ASSERT(lambda_node);
	if (!lambda_node)
		return;
	
	jive_region * function_region = lambda_node->base.inputs[0]->origin->node->region;
	jive_node * head = function_region->top;
	jive_node * tail = function_region->bottom;
	
	jive_substitution_map * substitution = jive_substitution_map_create(apply_node->graph->context);
	
	size_t n;
	for(n = 0; n < lambda_node->attrs.function_type.narguments; n++) {
		jive_gate * gate = lambda_node->attrs.argument_gates[n];
		jive_output * output = jive_node_get_gate_output(head, gate);
		jive_substitution_map_add_output(substitution, output, apply_node->inputs[n+1]->origin);
	}
	
	jive_region_copy_substitute(function_region,
		apply_node->region, substitution, false, false);
	
	for(n = 0; n < lambda_node->attrs.function_type.nreturns; n++) {
		jive_gate * gate = lambda_node->attrs.return_gates[n];
		jive_input * input = jive_node_get_gate_input(tail, gate);
		jive_output * substituted = jive_substitution_map_lookup_output(substitution, input->origin);
		jive_output * output = apply_node->outputs[n];
		jive_output_replace(output, substituted);
	}
	
	jive_substitution_map_destroy(substitution);
}
