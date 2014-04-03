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
create_testgraph_split(jive_context * context)
{
	/* register usage counts are satisfied locally, but there
	is a "critical value" where no global assignment of one
	register is possible */
	jive_graph * graph = jive_graph_create(context);
	jive_subroutine subroutine = jive_testarch_subroutine_begin(graph,
		0, NULL,
		0, NULL);
	jive_output * memstate = jive_subroutine_simple_get_global_state(subroutine);
	const jive_type * memtype = jive_output_get_type(memstate);
	jive_node * enter_mux = jive_state_split(memtype, memstate, 1);
	jive_node * leave_mux = jive_state_merge(memtype, 1, enter_mux->outputs)->node;
	jive_subroutine_simple_set_global_state(subroutine, leave_mux->outputs[0]);
	
	jive_gate * arg1_gate = jive_register_class_create_gate(&jive_testarch_regcls_gpr, graph, "arg1");
	jive_gate * arg2_gate = jive_register_class_create_gate(&jive_testarch_regcls_r3, graph, "arg2");
	
	jive_gate * retval_gate = jive_register_class_create_gate(&jive_testarch_regcls_gpr, graph, "retval");
	
	jive_output * arg1 = jive_node_gate_output(enter_mux, arg1_gate);
	jive_output * arg2 = jive_node_gate_output(enter_mux, arg2_gate);
	jive_output * tmparray0[] = {arg1};
	
	jive_output * critical_value = jive_instruction_node_create(
		subroutine.region,
		&jive_testarch_instr_move_gpr,
		tmparray0, NULL)->outputs[0];
	jive_output * tmparray1[] = {critical_value};
	
	jive_output * tmp = jive_instruction_node_create(
		subroutine.region,
		&jive_testarch_instr_setr1,
		tmparray1, NULL)->outputs[0];
	jive_output * tmparray2[] = {tmp};
	tmp = jive_instruction_node_create(
		subroutine.region,
		&jive_testarch_instr_setr2,
		tmparray2, NULL)->outputs[0];
	jive_output * tmparray3[] = {critical_value, tmp};
	tmp = jive_instruction_node_create(
		subroutine.region,
		&jive_testarch_instr_add,
		tmparray3, NULL)->outputs[0];
	
	jive_node_gate_input(leave_mux, retval_gate, tmp);
	jive_node_gate_input(leave_mux, arg2_gate, arg2);
	
	jive_graph_export(graph, jive_subroutine_end(subroutine)->outputs[0]);
	
	return graph;
}

static jive_graph *
create_testgraph_emerg_split(jive_context * context)
{
	/* creates variables with the following lifetimes:
	 a   c
	 |   |
	 |     d
	 |     |
	 |       e
	 |       |
	   b     |
	   |     |
	   |       f
	   |       |
	
	with the following register constraints:
	a - r1
	b - r2
	c,d,e,f - each either r1 or r2
	
	Due to number of intefering values, a will be assigned
	r1 first (denying r1 to e), and subsequently b will
	be considered. Even after life range split, assigning
	the only possible choice r2 to b, it leave_muxs no register
	for e over its lifetime.
	
	The register allocator must allow r2 for b after splitting
	the lifetime such that no use of e falls within its
	lifetime. */
	
	jive_graph * graph = jive_graph_create(context);
	jive_subroutine subroutine = jive_testarch_subroutine_begin(graph,
		0, NULL,
		0, NULL);
	jive_output * memstate = jive_subroutine_simple_get_global_state(subroutine);
	const jive_type * memtype = jive_output_get_type(memstate);
	jive_node * enter_mux = jive_state_split(memtype, memstate, 1);
	jive_node * leave_mux = jive_state_merge(memtype, 1, enter_mux->outputs)->node;
	jive_subroutine_simple_set_global_state(subroutine, leave_mux->outputs[0]);
	
	jive_gate * arg3_gate = jive_register_class_create_gate(&jive_testarch_regcls_r3, graph, "arg2");
	jive_output * arg3 = jive_node_gate_output(enter_mux, arg3_gate);
	jive_node_gate_input(leave_mux, arg3_gate, arg3);
	
	jive_gate * cls2_gate = jive_register_class_create_gate(&jive_testarch_regcls_r2, graph, "cls2");
	jive_output * arg1 = jive_node_gate_output(enter_mux, cls2_gate);
	jive_output * tmparray4[] = {arg1};
	jive_node * op3 = jive_instruction_node_create(
		subroutine.region,
		&jive_testarch_instr_setr1,
		tmparray4, NULL);
	jive_node_gate_input(leave_mux, jive_register_class_create_gate(&jive_testarch_regcls_r1, graph, "cls1"), op3->outputs[0]);
	
	jive_gate * arg2_gate = jive_register_class_create_gate(&jive_testarch_regcls_gpr, graph, "1or2");
	jive_output * arg2 = jive_node_gate_output(enter_mux, arg2_gate);
	jive_output * tmparray5[] = {arg2};
	jive_node * op1 = jive_instruction_node_create(
		subroutine.region,
		&jive_testarch_instr_move_gpr,
		tmparray5, NULL);
	jive_output * tmparray6[] = {op1->outputs[0]};
	jive_node * op2 = jive_instruction_node_create(
		subroutine.region,
		&jive_testarch_instr_move_gpr,
		tmparray6, NULL);
	jive_output * tmparray7[] = {op2->outputs[0]};
	jive_node * op4 = jive_instruction_node_create(
		subroutine.region,
		&jive_testarch_instr_move_gpr,
		tmparray7, NULL);
	jive_node_gate_input(leave_mux, jive_register_class_create_gate(&jive_testarch_regcls_gpr, graph, "cls1"), op4->outputs[0]);
	
	JIVE_DECLARE_STATE_TYPE(state_type);
	
	jive_node_add_input(op3, state_type, jive_node_add_output(op2, state_type));
	jive_node_add_input(op4, state_type, jive_node_add_output(op3, state_type));
	
	jive_graph_export(graph, jive_subroutine_end(subroutine)->outputs[0]);
	
	return graph;
}

typedef jive_graph * (*creator_function_t)(jive_context *);

static const creator_function_t tests[] = {
	create_testgraph_split,
	create_testgraph_emerg_split
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

JIVE_UNIT_TEST_REGISTER("regalloc/test-split", test_main);
