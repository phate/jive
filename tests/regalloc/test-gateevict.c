/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>
#include <stdio.h>

#include <jive/arch/subroutine/nodes.h>
#include <jive/regalloc.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/view.h>
#include <jive/vsdg.h>

#include "testarch.h"

static jive_graph *
create_testgraph_gateevict(jive_context * context)
{
	jive_graph * graph = jive_graph_create(context);
	jive_subroutine * subroutine = jive_testarch_subroutine_create(
		graph->root_region,
		0, NULL,
		0, NULL
	);
	jive_node * enter = &subroutine->enter->base;
	jive_node * leave = &subroutine->leave->base;
	
	jive_gate * arg1_gate = jive_register_class_create_gate(&jive_testarch_regcls_r1, graph, "arg1");
	jive_gate * retval_gate = jive_register_class_create_gate(&jive_testarch_regcls_r1, graph, "arg1");
	
	jive_output * arg1 = jive_node_gate_output(enter, arg1_gate);
	
	arg1 = jive_instruction_node_create(
		subroutine->region,
		&jive_testarch_instr_move_gpr,
		(jive_output *[]) {arg1}, NULL)->outputs[0];
	
	jive_gate * passthrough = jive_register_class_create_gate(&jive_testarch_regcls_gpr, graph, "passthrough");
	jive_gate * r1g = jive_register_class_create_gate(&jive_testarch_regcls_r1, graph, "r1");
	jive_gate * r2g = jive_register_class_create_gate(&jive_testarch_regcls_r2, graph, "r2");
	jive_gate * r3g = jive_register_class_create_gate(&jive_testarch_regcls_r3, graph, "r3");
	
	jive_node * nop1 = jive_instruction_node_create(
		subroutine->region,
		&JIVE_PSEUDO_NOP,
		NULL, NULL);
	jive_node * nop2 = jive_instruction_node_create(
		subroutine->region,
		&JIVE_PSEUDO_NOP,
		NULL, NULL);
	jive_node * nop3 = jive_instruction_node_create(
		subroutine->region,
		&JIVE_PSEUDO_NOP,
		NULL, NULL);
	
	jive_node_gate_input(nop1, passthrough, arg1);
	jive_node_gate_input(nop2, passthrough, jive_node_gate_output(nop1, passthrough));
	jive_node_gate_input(nop3, passthrough, jive_node_gate_output(nop2, passthrough));
	
	jive_output * result = jive_instruction_node_create(
		subroutine->region,
		&jive_testarch_instr_move_gpr,
		(jive_output *[]) {jive_node_gate_output(nop3, passthrough)}, NULL)->outputs[0];
	
	jive_node_gate_input(leave, retval_gate, result);
	
	jive_node_gate_output(nop1, r1g);
	jive_node_gate_output(nop2, r2g);
	jive_node_gate_output(nop3, r3g);
	
	return graph;
}

typedef jive_graph * (*creator_function_t)(jive_context *);

static const creator_function_t tests[] = {
	create_testgraph_gateevict
};

static int test_main(void)
{
	setlocale(LC_ALL, "");
	
	jive_context * context = jive_context_create();
	
	size_t n;
	for (n = 0; n < sizeof(tests)/sizeof(tests[0]); n++) {
		fprintf(stderr, "%zd\n", n);
		jive_graph * graph = tests[n](context);
		jive_view(graph, stdout);
		jive_shaped_graph * shaped_graph = jive_regalloc(graph);
		jive_view(graph, stdout);
		jive_shaped_graph_destroy(shaped_graph);
		jive_graph_destroy(graph);
	}
	
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("regalloc/test-gateevict", test_main);
