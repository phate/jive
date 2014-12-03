/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>
#include <stdio.h>

#include <jive/regalloc/color.h>
#include <jive/regalloc/regreuse.h>
#include <jive/regalloc/shape.h>
#include <jive/view.h>
#include <jive/vsdg.h>

#include "testarch.h"

static int test_main(void)
{
	setlocale(LC_ALL, "");
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_region * region = graph->root_region;
	
	jive_node * enter, * add1, * add2, * leave;
	
	enter = (jive_node *) jive_instruction_node_create(region,
		&jive_testarch_instructions[instr_nop],
		NULL, NULL);
	
	leave = (jive_node *) jive_instruction_node_create(region,
		&jive_testarch_instructions[instr_nop],
		NULL, NULL);
	
	jive::gate * stackptr_var;
	stackptr_var = jive_regcls_create_gate( &jive_testarch_regcls_r4, graph, "stackptr" );
	
	jive_node_gate_input(leave, stackptr_var, jive_node_gate_output(enter, stackptr_var));
	jive::output * arg1 = jive_node_gate_output(enter,
		jive_regcls_create_gate( &jive_testarch_regcls_regs, graph, "arg1"));
	jive::output * arg2 = jive_node_gate_output(enter,
		jive_regcls_create_gate( &jive_testarch_regcls_regs, graph, "arg1"));
	jive::output * arg3 = jive_node_gate_output(enter,
		jive_regcls_create_gate( &jive_testarch_regcls_regs, graph, "arg1"));
	jive::output * tmparray0[] = {arg1, arg2};
	
	add1 = (jive_node *) jive_instruction_node_create(region,
		&jive_testarch_instructions[instr_add],
		tmparray0, NULL);
	jive::output * tmparray1[] = {add1->outputs[0], arg3};
	
	add2 = (jive_node *) jive_instruction_node_create(region,
		&jive_testarch_instructions[instr_add],
		tmparray1, NULL);
	
	jive_node_gate_input(leave, jive_regcls_create_gate( &jive_testarch_regcls_r1, graph, "ret1"),
		add2->outputs[0]);
	
	jive_view(graph, stderr);
	
	jive_regalloc_shape(graph);
	
	jive_cut * cut = region->cuts.first;
	assert((cut->nodes.first == cut->nodes.last) && (cut->nodes.first->node == enter));
	cut = cut->region_cuts_list.next;
	assert((cut->nodes.first == cut->nodes.last) && (cut->nodes.first->node == add1));
	cut = cut->region_cuts_list.next;
	assert((cut->nodes.first == cut->nodes.last) && (cut->nodes.first->node == add2));
	cut = cut->region_cuts_list.next;
	assert((cut->nodes.first == cut->nodes.last) && (cut->nodes.first->node == leave));
	cut = cut->region_cuts_list.next;
	assert(cut == 0);
	
	jive_regalloc_color(graph);
	
	jive_regalloc_regreuse(graph);
	
	jive_view(graph, stderr);
	
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	return 0;
}

JIVE_UNIT_TEST_REGISTER("regalloc/simple-shaping", test_main);
