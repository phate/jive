#include "test-registry.h"

#include <assert.h>
#include <stdio.h>
#include <locale.h>

#include <jive/regalloc.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/view.h>
#include <jive/vsdg.h>

#include "testarch.h"

static jive_graph *
create_testgraph_postop_xfer(jive_context * context)
{
	/* requires post-op transfer to satisfy register constraints */
	jive_graph * graph = jive_graph_create(context);
	
	jive_subroutine * subroutine = jive_testarch_subroutine_create(
		graph->root_region,
		2, (const jive_argument_type[]) { jive_argument_int, jive_argument_int },
		0, NULL
	);
	jive_node * leave = &subroutine->leave->base;
	
	jive_gate * retval_gate = jive_register_class_create_gate(&jive_testarch_regcls[cls_r3], graph, "retval");
	
	jive_output * arg1 = jive_subroutine_value_parameter(subroutine, 0);
	jive_output * arg2 = jive_subroutine_value_parameter(subroutine, 1);
	jive_output * retval = jive_instruction_node_create(
		subroutine->region,
		&jive_testarch_instructions[sub_gpr_index],
		(jive_output *[]) {arg1, arg2}, NULL)->outputs[0];
	jive_node_gate_input(leave, retval_gate, retval);
	
	return graph;
}

static jive_graph *
create_testgraph_preop_xfer(jive_context * context)
{
	/* requires pre-op transfer to satisfy register constraints */
	jive_graph * graph = jive_graph_create(context);
	
	jive_subroutine * subroutine = jive_testarch_subroutine_create(
		graph->root_region,
		2, (const jive_argument_type[]) { jive_argument_int, jive_argument_int },
		0, NULL
	);
	jive_node * leave = &subroutine->leave->base;
	
	jive_gate * retval_gate = jive_register_class_create_gate(&jive_testarch_regcls[cls_r3], graph, "retval");
	jive_gate * retval2_gate = jive_register_class_create_gate(&jive_testarch_regcls[cls_r1], graph, "retval2");
	
	jive_output * arg1 = jive_subroutine_value_parameter(subroutine, 0);
	jive_output * arg2 = jive_subroutine_value_parameter(subroutine, 1);
	jive_output * retval = jive_instruction_node_create(
		subroutine->region,
		&jive_testarch_instructions[sub_gpr_index],
		(jive_output *[]) {arg1, arg2}, NULL)->outputs[0];
	jive_node_gate_input(leave, retval_gate, retval);
	jive_node_gate_input(leave, retval2_gate, arg1);
	
	return graph;
}

static jive_graph *
create_testgraph_preop_aux_xfer(jive_context * context)
{
	/* requires pre-op transfer to auxiliary register to satisfy register constraints */
	jive_graph * graph = jive_graph_create(context);
	
	jive_subroutine * subroutine = jive_testarch_subroutine_create(
		graph->root_region,
		2, (const jive_argument_type[]) { jive_argument_int, jive_argument_int },
		0, NULL
	);
	jive_node * leave = &subroutine->leave->base;
	
	jive_gate * retval_gate = jive_register_class_create_gate(&jive_testarch_regcls[cls_r2], graph, "retval");
	jive_gate * retval2_gate = jive_register_class_create_gate(&jive_testarch_regcls[cls_r1], graph, "retval2");
	
	jive_output * arg1 = jive_subroutine_value_parameter(subroutine, 0);
	jive_output * arg2 = jive_subroutine_value_parameter(subroutine, 1);
	jive_output * retval = jive_instruction_node_create(
		subroutine->region,
		&jive_testarch_instructions[sub_gpr_index],
		(jive_output *[]) {arg1, arg2}, NULL)->outputs[0];
	jive_node_gate_input(leave, retval_gate, retval);
	jive_node_gate_input(leave, retval2_gate, arg1);
	
	return graph;
}

static jive_graph *
create_testgraph_preop_aux_xfer_shaper(jive_context * context)
{
	/* requires pre-op transfer to auxiliary register to satisfy register constraints
	additionally, the auxiliary register must be reserved by the shaper */
	jive_graph * graph = jive_graph_create(context);
	
	jive_subroutine * subroutine = jive_testarch_subroutine_create(
		graph->root_region,
		2, (const jive_argument_type[]) { jive_argument_int, jive_argument_int },
		0, NULL
	);
	jive_node * enter = &subroutine->enter->base;
	jive_node * leave = &subroutine->leave->base;
	
	jive_gate * save_r3_gate = jive_register_class_create_gate(&jive_testarch_regcls[cls_r3], graph, "save_r3");
	jive_gate * retval_gate = jive_register_class_create_gate(&jive_testarch_regcls[cls_r2], graph, "retval");
	jive_gate * retval2_gate = jive_register_class_create_gate(&jive_testarch_regcls[cls_r1], graph, "retval2");
	
	jive_output * arg1 = jive_subroutine_value_parameter(subroutine, 0);
	jive_output * arg2 = jive_subroutine_value_parameter(subroutine, 1);
	jive_output * retval = jive_instruction_node_create(
		subroutine->region,
		&jive_testarch_instructions[sub_gpr_index],
		(jive_output *[]) {arg1, arg2}, NULL)->outputs[0];
	jive_node_gate_input(leave, retval_gate, retval);
	jive_node_gate_input(leave, retval2_gate, arg1);
	
	jive_node_gate_input(leave, save_r3_gate, jive_node_gate_output(enter, save_r3_gate));
	
	return graph;
}

typedef jive_graph * (*creator_function_t)(jive_context *);

static const creator_function_t tests[] = {
	create_testgraph_postop_xfer,
	create_testgraph_preop_xfer,
	create_testgraph_preop_aux_xfer,
	create_testgraph_preop_aux_xfer_shaper,
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

JIVE_UNIT_TEST_REGISTER("regalloc/test-twoop", test_main);
