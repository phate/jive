/*
 * Copyright 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/function/fctapply.h>

#include <jive/vsdg/node-private.h>
#include <jive/types/function/fcttype.h>

#include <string.h>

static void
jive_apply_node_init_(jive_apply_node * self, struct jive_region * region, jive_output * function,
	size_t narguments, jive_output * const arguments[]);

static jive_node *
jive_apply_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

static char *
jive_apply_node_get_label_(const jive_node * self_);

const jive_node_class JIVE_APPLY_NODE = {
	.parent = &JIVE_NODE,
	.name = "APPLY",
	.fini = jive_node_fini_, /* inherit */
	.get_default_normal_form = jive_node_get_default_normal_form_, /* inherit */
	.get_label = jive_apply_node_get_label_, /* override */
	.get_attrs = jive_node_get_attrs_, /* inherit */
	.match_attrs = jive_node_match_attrs_, /* inherit */
	.create = jive_apply_node_create_, /* override */
	.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
};

static char *
jive_apply_node_get_label_(const jive_node * self_)
{
	return strdup("APPLY");
}

static jive_node *
jive_apply_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[])
{
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
	if (function->class_ != &JIVE_FUNCTION_OUTPUT) {
		jive_context_fatal_error(region->graph->context, "Type mismatch: need 'function' type as input to 'apply' node");
	}
	jive_function_output * fct = (jive_function_output *) function;

	if (fct->type.narguments != narguments) {
		jive_context_fatal_error(region->graph->context, "Type mismatch: number of parameters to function does not match signature");
	}

	jive_output * args[narguments+1];
	const jive_type * argument_types[narguments+1];
	args[0] = function;
	argument_types[0] = &fct->type.base.base; 
 
	size_t i;
	for(i = 0; i < fct->type.narguments; i++){
		argument_types[i+1] = fct->type.argument_types[i];
		args[i+1] = arguments[i];
	}
	
	jive_node_init_(self, region,
		narguments + 1, argument_types, args,
		fct->type.nreturns, (const jive_type * const *) fct->type.return_types);
}

jive_node *
jive_apply_node_create(struct jive_region * region, jive_output * function,
	size_t narguments, jive_output * const arguments[])
{
	jive_apply_node * node = jive_context_malloc(region->graph->context, sizeof( * node));

	node->class_ = &JIVE_APPLY_NODE;
	jive_apply_node_init_(node, region, function, narguments, arguments);

	return node; 
}
