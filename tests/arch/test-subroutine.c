/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/view.h>
#include "testarch.h"

static int test_main(void)
{
	setlocale(LC_ALL, "");
	
	jive_context * context = jive_context_create();
	
	jive_graph * graph = jive_graph_create(context);
	
	jive_subroutine * subroutine = jive_testarch_subroutine_create(graph->root_region,
		4, (jive_argument_type []){jive_argument_long, jive_argument_long, jive_argument_long, jive_argument_long},
		1, (jive_argument_type []){jive_argument_long});
	
	jive_output * arg1 = jive_subroutine_value_parameter(subroutine, 0);
	jive_output * arg2 = jive_subroutine_value_parameter(subroutine, 1);
	jive_output * arg3 = jive_subroutine_value_parameter(subroutine, 2);
	
	jive_output * s1 = jive_instruction_node_create(
		subroutine->region,
		&jive_testarch_instr_add,
		(jive_output *[]) {arg1, arg2}, NULL)->outputs[0];
	jive_output * s2 = jive_instruction_node_create(
		subroutine->region,
		&jive_testarch_instr_add,
		(jive_output *[]) {s1, arg3}, NULL)->outputs[0];
	jive_subroutine_value_return(subroutine, 0, s2);
	
	jive_view(graph, stdout);
	
	jive_context * context2 = jive_context_create();
	jive_graph * graph2 = jive_graph_copy(graph, context2);
	
	jive_subroutine_node * anchor2 = jive_subroutine_node_cast(graph2->bottom.first);
	assert(anchor2);
	jive_subroutine * sub2 = anchor2->attrs.subroutine;
	assert(sub2);
	assert(sub2->nparameters == 4);
	assert(sub2->parameters[0] && jive_node_get_gate_output(&sub2->enter->base, sub2->parameters[0]));
	assert(sub2->parameters[1] && jive_node_get_gate_output(&sub2->enter->base, sub2->parameters[1]));
	assert(sub2->parameters[2] && jive_node_get_gate_output(&sub2->enter->base, sub2->parameters[2]));
	assert(sub2->parameters[3]);
	assert(jive_node_get_gate_output(&sub2->enter->base, sub2->parameters[3]) == NULL);
	
	jive_graph_destroy(graph);
	
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);
	
	jive_view(graph2, stdout);
	
	jive_graph_destroy(graph2);
	assert(jive_context_is_empty(context2));
	jive_context_destroy(context2);
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-subroutine", test_main);
