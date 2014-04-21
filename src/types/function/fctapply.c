/*
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/function/fctapply.h>

#include <jive/types/function/fcttype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>

#include <string.h>

static void
jive_apply_node_init_(jive_apply_node * self, struct jive_region * region, jive_output * function,
	size_t narguments, jive_output * const arguments[]);

static jive_node *
jive_apply_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

const jive_node_class JIVE_APPLY_NODE = {
	parent : &JIVE_NODE,
	name : "APPLY",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_node_get_default_normal_form_, /* inherit */
	get_label : jive_node_get_label_, /* inherit */
	get_attrs : jive_node_get_attrs_, /* inherit */
	match_attrs : jive_node_match_attrs_, /* inherit */
	check_operands : NULL,
	create : jive_apply_node_create_, /* override */
};

static jive_node *
jive_apply_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands > 0);
	return jive_apply_node_create(region, operands[0], noperands - 1, &operands[1]);
}

static void
jive_apply_node_init_(
	jive_apply_node * self,
	struct jive_region * region,
	jive_output * function,
	size_t narguments,
	jive_output * const arguments[])
{
	if (!jive_output_isinstance(function, &JIVE_FUNCTION_OUTPUT)) {
		jive_context_fatal_error(region->graph->context, "Type mismatch: need 'function' type as input to 'apply' node");
	}
	jive_function_output * fct = (jive_function_output *) function;

	if (fct->narguments() != narguments) {
		jive_context_fatal_error(region->graph->context, "Type mismatch: number of parameters to function does not match signature");
	}

	jive_output * args[narguments+1];
	const jive_type * argument_types[narguments+1];
	args[0] = function;
	argument_types[0] = &fct->type();
 
	size_t i;
	for(i = 0; i < fct->narguments(); i++){
		argument_types[i+1] = fct->argument_type(i);
		args[i+1] = arguments[i];
	}
	
	const jive_type * return_types[fct->nreturns()];
	for (i = 0; i < fct->nreturns(); i++)
		return_types[i] = fct->return_type(i);

	jive_node_init_(self, region,
		narguments + 1, argument_types, args,
		fct->nreturns(), return_types);
}

jive_node *
jive_apply_node_create(struct jive_region * region, jive_output * function,
	size_t narguments, jive_output * const arguments[])
{
	jive_apply_node * node = new jive_apply_node;

	node->class_ = &JIVE_APPLY_NODE;
	jive_apply_node_init_(node, region, function, narguments, arguments);

	return node;
}

void
jive_apply_create(jive_output * function, size_t narguments, jive_output * const arguments[],
	jive_output * results[])
{
	size_t n;
	size_t noperands = narguments + 1;
	jive_output * operands[noperands];
	for (n = 0; n < narguments; n++)
		operands[n] = arguments[n];
	operands[n] = function; 

	jive_region * region = jive_region_innermost(noperands, operands);
	jive_node * apply = jive_apply_node_create(region, function, narguments, arguments);

	for (n = 0; n < apply->noutputs; n++)
		results[n] = apply->outputs[n];
}
