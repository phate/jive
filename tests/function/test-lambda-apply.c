/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>
#include <jive/types/bitstring.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/function.h>
#include <jive/vsdg/node-private.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");
	jive_context * ctx = jive_context_create();
	
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_region * function = jive_function_region_create(graph->root_region);
	
	JIVE_DECLARE_BITSTRING_TYPE(int32, 32);
	jive_gate * arg1_gate = jive_type_create_gate(int32, graph, "arg1");
	jive_gate * arg2_gate = jive_type_create_gate(int32, graph, "arg2");
	jive_gate * ret_gate = jive_type_create_gate(int32, graph, "ret");
	
	jive_output * arg1 = jive_node_gate_output(function->top, arg1_gate);
	jive_output * arg2 = jive_node_gate_output(function->top, arg2_gate);
	jive_output * sum = jive_bitsum(2, (jive_output *[]){arg1, arg2});
	jive_node_gate_input(function->bottom, ret_gate, sum);
	
	jive_output * lambda_expr = jive_lambda_create(function);
	
	jive_output * c0 = jive_bitconstant(graph, 32, "01010100000000000000000000000000");
	jive_output * c1 = jive_bitconstant(graph, 32, "10010010000000000000000000000000");
	
	jive_node * apply_node = jive_apply_node_create(graph->root_region,
		lambda_expr, 2, (jive_output *[]){c0, c1});
	
	assert(jive_type_equals(int32, jive_output_get_type(apply_node->outputs[0])));
	
	jive_node * interest = jive_node_create(
		graph->root_region,
		1, (const jive_type *[]){int32}, apply_node->outputs,
		0, 0);
	
	jive_node_reserve(interest);
	
	jive_view(graph, stderr);
	
	jive_inline_lambda_apply(apply_node);
	jive_graph_prune(graph);
	
	jive_node * test_sum = interest->inputs[0]->origin->node;
	assert(jive_node_isinstance(test_sum, &JIVE_BITSUM_NODE));
	assert(test_sum->ninputs == 2);
	assert(test_sum->inputs[0]->origin == c0);
	assert(test_sum->inputs[1]->origin == c1);
	
	jive_view(graph, stderr);
	
	jive_graph_destroy(graph);
	
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	return 0;
}

JIVE_UNIT_TEST_REGISTER("function/test-lambda-apply", test_main);
