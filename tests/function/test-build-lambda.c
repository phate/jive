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

static int test_main(void)
{
	setlocale(LC_ALL, "");
	jive_context * ctx = jive_context_create();
	
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_region * function = jive_function_region_create(graph->root_region);

	jive_node * top = jive_region_get_top_node(function);
	jive_node * bottom = jive_region_get_bottom_node(function);

	assert(top == function->top);
	assert(bottom = function->bottom);
	
	JIVE_DECLARE_BITSTRING_TYPE(int32, 32);
	jive_gate * arg1_gate = jive_type_create_gate(int32, graph, "arg1");
	jive_gate * arg2_gate = jive_type_create_gate(int32, graph, "arg2");
	jive_gate * ret_gate = jive_type_create_gate(int32, graph, "ret");
	
	jive_output * arg1 = jive_node_gate_output(top, arg1_gate);
	jive_output * arg2 = jive_node_gate_output(top, arg2_gate);
	jive_output * sum = jive_bitsum(2, (jive_output *[]){arg1, arg2});
	jive_node_gate_input(bottom, ret_gate, sum);
	
	jive_node * lambda_node = jive_lambda_node_create(function);
	jive_node_reserve(lambda_node);
	
	jive_function_type ftype;
	jive_function_type_init(&ftype, ctx, 2, (const jive_type *[]){int32, int32}, 1, (const jive_type *[]){int32});
	
	assert(jive_type_equals(&ftype.base.base, jive_output_get_type(lambda_node->outputs[0])));
	
	jive_function_type_fini(&ftype);
	
	jive_view(graph, stderr);
	
	jive_graph_destroy(graph);
	
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	return 0;
}

JIVE_UNIT_TEST_REGISTER("function/test-build-lambda", test_main);
