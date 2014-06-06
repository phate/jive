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
create_testgraph(jive_context * ctx)
{
	jive_graph * graph = jive_graph_create(ctx);
	const jive_argument_type tmparray0[] = { jive_argument_int };
	const jive_argument_type tmparray1[] = { jive_argument_int };
	jive_subroutine subroutine = jive_testarch_subroutine_begin(graph,
		1, tmparray0,
		1, tmparray1
	);

	jive_output * memstate = jive_subroutine_simple_get_global_state(subroutine);
	const jive::base::type * memtype = &memstate->type();
	jive_node * enter_mux = jive_state_split(memtype, memstate, 1);
	jive_node * leave_mux = jive_state_merge(memtype, 1, enter_mux->outputs)->node();
	jive_subroutine_simple_set_global_state(subroutine, leave_mux->outputs[0]);
	
	jive_region * region = subroutine.region;
	
	jive_gate * callee_saved_r2 = jive_register_class_create_gate(&jive_testarch_regcls_r2, graph, "saved_r2");
	jive_node_gate_input(leave_mux, callee_saved_r2, jive_node_gate_output(enter_mux, callee_saved_r2));
	
	jive_gate * callee_saved_r3 = jive_register_class_create_gate(&jive_testarch_regcls_r3, graph, "saved_r0");
	jive_node_gate_input(leave_mux, callee_saved_r3, jive_node_gate_output(enter_mux, callee_saved_r3));
	
	jive_output * arg1 = jive_subroutine_simple_get_argument(subroutine, 0);
	jive_output * tmparray2[] = {arg1};
	int64_t  tmparray3[] = {0};
	
	jive_output * v1 = jive_instruction_node_create(
		region,
		&jive_testarch_instr_load_disp,
		tmparray2, tmparray3)->outputs[0];
	jive_output * tmparray4[] = {arg1};
	int64_t  tmparray5[] = {4};
	
	jive_output * v2 = jive_instruction_node_create(
		region,
		&jive_testarch_instr_load_disp,
		tmparray4, tmparray5)->outputs[0];
jive_output * tmparray6[] = {v1, v2};

	jive_output * sum = jive_instruction_node_create(
		region,
		&jive_testarch_instr_add,
		tmparray6, 0)->outputs[0];
	
	jive_subroutine_simple_set_result(subroutine, 0, sum);
	
	jive_graph_export(graph, jive_subroutine_end(subroutine)->outputs[0]);
	
	return graph;
}

static int test_main(void)
{
	setlocale(LC_ALL, "");
	jive_context * ctx = jive_context_create();
	
	jive_graph * graph = create_testgraph(ctx);
	jive_view(graph, stdout);
	jive_shaped_graph * shaped_graph = jive_regalloc(graph);
	jive_view(graph, stdout);
	jive_shaped_graph_destroy(shaped_graph);
	jive_graph_destroy(graph);
	
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("regalloc/test-shape-split", test_main);
