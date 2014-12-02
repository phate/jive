/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>
#include <stdio.h>

#include <jive/regalloc.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/view.h>
#include <jive/vsdg.h>

#include "testarch.h"

static jive_graph *
create_testgraph()
{
	jive_graph * graph = jive_graph_create();
	
	jive_node * enter = jive_instruction_node_create(
		graph->root_region,
		&jive_testarch_instr_nop,
		0, 0);
	jive_node * leave = jive_instruction_node_create(
		graph->root_region,
		&jive_testarch_instr_nop,
		0, 0);
	
	jive::gate * stackptr_gate = jive_register_class_create_gate(&jive_testarch_regcls_r3, graph,
		"stackptr");
	stackptr_gate->may_spill = false;
	
	jive::output * stackptr = jive_node_gate_output(enter, stackptr_gate);
	jive_node_gate_input(leave, stackptr_gate, stackptr);
	
	jive::output * arg1 = jive_node_gate_output(enter, jive_register_class_create_gate(
		&jive_testarch_regcls_gpr, graph, "arg1"));
	jive::output * arg2 = jive_node_gate_output(enter, jive_register_class_create_gate(
		&jive_testarch_regcls_gpr, graph, "arg2"));
	jive::output * arg3 = jive_node_gate_output(enter, jive_register_class_create_gate(
		&jive_testarch_regcls_gpr, graph, "arg3"));
	
	jive::output * sum;
	jive::output * tmparray0[] = {arg1, arg2};
	sum = jive_instruction_node_create(
		graph->root_region,
		&jive_testarch_instr_add,
		tmparray0, 0)->outputs[0];
	jive::output * tmparray1[] = {sum, arg3};
	sum = jive_instruction_node_create(
		graph->root_region,
		&jive_testarch_instr_add,
		tmparray1, 0)->outputs[0];
	
	jive_node_gate_input(leave, jive_register_class_create_gate(&jive_testarch_regcls_gpr, graph,
		"ret1"), sum);
	
	return graph;
}

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = create_testgraph();
	jive_view(graph, stdout);
	jive_shaped_graph * shaped_graph = jive_regalloc(graph);
	jive_view(graph, stdout);
	jive_shaped_graph_destroy(shaped_graph);
	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("regalloc/test-simple", test_main);
