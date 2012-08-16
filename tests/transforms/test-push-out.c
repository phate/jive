/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>
#include <jive/types/bitstring.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/types/function/fctapply.h>
#include <jive/types/function/fctlambda.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");
	jive_context * ctx = jive_context_create();
	
	jive_graph * graph = jive_graph_create(ctx);
	
	JIVE_DECLARE_BITSTRING_TYPE(int32, 32);
	
	jive_region * outer_function = jive_function_region_create(graph->root_region);
	jive_region * inner_function = jive_function_region_create(outer_function);
	
	jive_gate * arg1_gate = jive_type_create_gate(int32, graph, "arg1");
	jive_gate * arg2_gate = jive_type_create_gate(int32, graph, "arg2");
	jive_output * arg1 = jive_node_gate_output(inner_function->top, arg1_gate);
	jive_output * arg2 = jive_node_gate_output(inner_function->top, arg2_gate);
	jive_output * sum = jive_bitsum(2, (jive_output *[]){arg1, arg2});
	jive_gate * inner_ret_gate = jive_type_create_gate(int32, graph, "inner_ret");
	jive_node_gate_input(inner_function->bottom, inner_ret_gate, sum);
	jive_node * inner_lambda = jive_lambda_node_create(inner_function);
	
	jive_gate * arg_gate = jive_type_create_gate(int32, graph, "arg");
	jive_output * arg = jive_node_gate_output(outer_function->top, arg_gate);
	jive_node * apply = jive_apply_node_create(outer_function, inner_lambda->outputs[0], 2, (jive_output *[]){arg, arg});
	jive_gate * outer_ret_gate = jive_type_create_gate(int32, graph, "outer_ret");
	jive_node_gate_input(outer_function->bottom, outer_ret_gate, apply->outputs[0]);
	jive_node * outer_lambda = jive_lambda_node_create(outer_function);
	
	jive_node_reserve(outer_lambda);
	
	jive_view(graph, stderr);
	
	assert(jive_node_can_move_outward(inner_lambda));
	jive_graph_push_outward(graph);
	assert(inner_lambda->region == graph->root_region);
	assert(inner_function->parent == graph->root_region);
	
	jive_view(graph, stderr);
	
	jive_graph_pull_inward(graph);
	/* must not be pulled back into lambda def region */
	assert(inner_lambda->region == graph->root_region);
	
	jive_view(graph, stderr);
	
	jive_graph_destroy(graph);
	
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	return 0;
}

JIVE_UNIT_TEST_REGISTER("transforms/test-push-out", test_main);
