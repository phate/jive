/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
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
create_testgraph_mismatch1(jive_context * context)
{
	jive_graph * graph = jive_graph_create(context);
	const jive_argument_type tmparray0[] = { jive_argument_int, jive_argument_int };
	const jive_argument_type tmparray1[] = { jive_argument_int };
	
	jive_subroutine subroutine = jive_testarch_subroutine_begin(graph,
		2, tmparray0,
		1, tmparray1
	);
	
	jive::output * arg2 = jive_subroutine_simple_get_argument(subroutine, 1);
	jive_subroutine_simple_set_result(subroutine, 0, arg2);
	jive_graph_export(graph, jive_subroutine_end(subroutine)->outputs[0]);

	return graph;
}

static jive_graph *
create_testgraph_mismatch2(jive_context * context)
{
	jive_graph * graph = jive_graph_create(context);
	const jive_argument_type tmparray2[] = { jive_argument_int, jive_argument_int };
	const jive_argument_type tmparray3[] = { jive_argument_int, jive_argument_int };
	
	jive_subroutine subroutine = jive_testarch_subroutine_begin(graph,
		2, tmparray2,
		2, tmparray3
	);
	
	jive::output * arg1 = jive_subroutine_simple_get_argument(subroutine, 0);
	jive::output * arg2 = jive_subroutine_simple_get_argument(subroutine, 1);
	jive_subroutine_simple_set_result(subroutine, 0, arg2);
	jive_subroutine_simple_set_result(subroutine, 1, arg1);
	
	jive_graph_export(graph, jive_subroutine_end(subroutine)->outputs[0]);

	return graph;
}

static jive_graph *
create_testgraph_mismatch3(jive_context * context)
{
	jive_graph * graph = jive_graph_create(context);
	jive_subroutine subroutine = jive_testarch_subroutine_begin(graph,
		0, NULL,
		0, NULL
	);
	jive::output * memstate = jive_subroutine_simple_get_global_state(subroutine);
	const jive::base::type * memtype = &memstate->type();
	jive_node * enter_mux = jive_state_split(memtype, memstate, 1);
	jive_node * leave_mux = jive_state_merge(memtype, 1, enter_mux->outputs)->node();
	jive_subroutine_simple_set_global_state(subroutine, leave_mux->outputs[0]);
	
	jive::output * arg1 = jive_node_gate_output(enter_mux, jive_register_class_create_gate(&jive_testarch_regcls_r1, graph, "cls1"));
	jive_node_gate_input(leave_mux, jive_register_class_create_gate(&jive_testarch_regcls_gpr, graph, "reg"), arg1);
	
	jive::output * arg2 = jive_node_gate_output(enter_mux, jive_register_class_create_gate(&jive_testarch_regcls_gpr, graph, "reg"));
	jive_node * node = jive_instruction_node_create(
		subroutine.region,
		&jive_testarch_instr_setr1,
		&arg2, NULL);
	arg2 = node->outputs[0];
	jive_node_gate_input(leave_mux, jive_register_class_create_gate(&jive_testarch_regcls_gpr, graph, "reg"), arg2);
	
	jive_graph_export(graph, jive_subroutine_end(subroutine)->outputs[0]);

	return graph;
}

static jive_graph *
create_testgraph_mismatch4(jive_context * context)
{
	jive_graph * graph = jive_graph_create(context);
	jive_subroutine subroutine = jive_testarch_subroutine_begin(graph,
		0, NULL,
		0, NULL
	);
	jive::output * memstate = jive_subroutine_simple_get_global_state(subroutine);
	const jive::base::type * memtype = &memstate->type();
	jive_node * enter_mux = jive_state_split(memtype, memstate, 1);
	jive_node * leave_mux = jive_state_merge(memtype, 1, enter_mux->outputs)->node();
	jive_subroutine_simple_set_global_state(subroutine, leave_mux->outputs[0]);
	
	jive::output * arg1 = jive_node_gate_output(enter_mux, jive_register_class_create_gate(&jive_testarch_regcls_gpr, graph, "reg"));
	jive_node_gate_input(leave_mux, jive_register_class_create_gate(&jive_testarch_regcls_r1, graph, "cls1"), arg1);
	
	jive::output * arg2 = jive_node_gate_output(enter_mux, jive_register_class_create_gate(&jive_testarch_regcls_gpr, graph, "reg"));
	jive_node * node = jive_instruction_node_create(
		subroutine.region,
		&jive_testarch_instr_setr1,
		&arg2, NULL);
	jive_node_gate_input(leave_mux, jive_register_class_create_gate(&jive_testarch_regcls_gpr, graph, "reg"), node->outputs[0]);
	
	jive_graph_export(graph, jive_subroutine_end(subroutine)->outputs[0]);

	return graph;
}

static jive_graph *
create_testgraph_mismatch5(jive_context * context)
{
	jive_graph * graph = jive_graph_create(context);
	jive_subroutine subroutine = jive_testarch_subroutine_begin(graph,
		0, NULL,
		0, NULL
	);
	jive::output * memstate = jive_subroutine_simple_get_global_state(subroutine);
	const jive::base::type * memtype = &memstate->type();
	jive_node * enter_mux = jive_state_split(memtype, memstate, 1);
	jive_node * leave_mux = jive_state_merge(memtype, 1, enter_mux->outputs)->node();
	jive_subroutine_simple_set_global_state(subroutine, leave_mux->outputs[0]);
	
	jive::output * arg1 = jive_node_gate_output(enter_mux, jive_register_class_create_gate(&jive_testarch_regcls_gpr, graph, "reg"));
	
	jive_node * tmp1 = jive_instruction_node_create(
		subroutine.region,
		&JIVE_PSEUDO_NOP,
		NULL, NULL);
	jive_node_gate_input(tmp1, jive_register_class_create_gate(&jive_testarch_regcls_r1, graph, "cls1"), arg1);
	jive::output * out1 = jive_node_gate_output(tmp1, jive_register_class_create_gate(&jive_testarch_regcls_gpr, graph, "regs"));
	
	jive_node * tmp2 = jive_instruction_node_create(
		subroutine.region,
		&JIVE_PSEUDO_NOP,
		NULL, NULL);
	jive_node_gate_input(tmp2, jive_register_class_create_gate(&jive_testarch_regcls_r2, graph, "cls2"), arg1);
	jive::output * out2 = jive_node_gate_output(tmp2, jive_register_class_create_gate(&jive_testarch_regcls_gpr, graph, "regs"));
	
	jive_node_gate_input(leave_mux, jive_register_class_create_gate(&jive_testarch_regcls_gpr, graph, "retval1"), out1);
	jive_node_gate_input(leave_mux, jive_register_class_create_gate(&jive_testarch_regcls_gpr, graph, "retval2"), out2);
	
	jive_graph_export(graph, jive_subroutine_end(subroutine)->outputs[0]);

	return graph;
}

static jive_graph *
create_testgraph_mismatch6(jive_context * context)
{
	jive_graph * graph = jive_graph_create(context);
	jive_subroutine subroutine = jive_testarch_subroutine_begin(graph,
		0, NULL,
		0, NULL
	);
	jive::output * memstate = jive_subroutine_simple_get_global_state(subroutine);
	const jive::base::type * memtype = &memstate->type();
	jive_node * enter_mux = jive_state_split(memtype, memstate, 1);
	jive_node * leave_mux = jive_state_merge(memtype, 1, enter_mux->outputs)->node();
	jive_subroutine_simple_set_global_state(subroutine, leave_mux->outputs[0]);
	
	jive_node * mid = jive_instruction_node_create(
		subroutine.region,
		&JIVE_PSEUDO_NOP,
		NULL, NULL);
	
	jive_node_gate_input(mid, jive_register_class_create_gate(&jive_testarch_regcls_gpr, graph, "anon"), jive_node_gate_output(enter_mux, jive_register_class_create_gate(&jive_testarch_regcls_gpr, graph, "anon")));
	
	jive_node_gate_input(leave_mux, jive_register_class_create_gate(&jive_testarch_regcls_gpr, graph, "anon"), jive_node_gate_output(enter_mux, jive_register_class_create_gate(&jive_testarch_regcls_gpr, graph, "anon")));
	jive_node_gate_input(leave_mux, jive_register_class_create_gate(&jive_testarch_regcls_gpr, graph, "anon"), jive_node_gate_output(enter_mux, jive_register_class_create_gate(&jive_testarch_regcls_gpr, graph, "anon")));
	
	jive_node_gate_input(leave_mux, jive_register_class_create_gate(&jive_testarch_regcls_r2, graph, "r2"), jive_node_gate_output(mid, jive_register_class_create_gate(&jive_testarch_regcls_gpr, graph, "anon")));
	jive_node_gate_output(mid, jive_register_class_create_gate(&jive_testarch_regcls_r2, graph, "r2"));
	
	jive_graph_export(graph, jive_subroutine_end(subroutine)->outputs[0]);

	return graph;
}

typedef jive_graph * (*creator_function_t)(jive_context *);

static const creator_function_t tests[] = {
	create_testgraph_mismatch1,
	create_testgraph_mismatch2,
	create_testgraph_mismatch3,
	create_testgraph_mismatch4,
	create_testgraph_mismatch5,
	create_testgraph_mismatch6
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

JIVE_UNIT_TEST_REGISTER("regalloc/test-regmismatch", test_main);
