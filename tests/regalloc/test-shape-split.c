#include "test-registry.h"

#include <assert.h>
#include <locale.h>
#include <stdio.h>

#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc.h>
#include <jive/view.h>
#include <jive/vsdg.h>

#include "testarch.h"

static jive_graph *
create_testgraph(jive_context * ctx)
{
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_subroutine * subroutine = jive_testarch_subroutine_create(
		graph->root_region,
		1, (const jive_argument_type[]) { jive_argument_int },
		1, (const jive_argument_type[]) { jive_argument_int }
	);
	
	jive_node * enter = &subroutine->enter->base;
	jive_node * leave = &subroutine->leave->base;
	jive_region * region = subroutine->region;
	
	jive_gate * callee_saved_r2 = jive_register_class_create_gate(&jive_testarch_regcls[cls_r2], graph, "saved_r2");
	jive_node_gate_input(leave, callee_saved_r2, jive_node_gate_output(enter, callee_saved_r2));
	
	jive_gate * callee_saved_r3 = jive_register_class_create_gate(&jive_testarch_regcls[cls_r3], graph, "saved_r0");
	jive_node_gate_input(leave, callee_saved_r3, jive_node_gate_output(enter, callee_saved_r3));
	
	jive_output * arg1 = jive_subroutine_value_parameter(subroutine, 0);
	
	jive_output * v1 = jive_instruction_node_create(
		region,
		&jive_testarch_instructions[load_disp_index],
		(jive_output *[]){arg1}, (int64_t []){0})->outputs[0];
	
	jive_output * v2 = jive_instruction_node_create(
		region,
		&jive_testarch_instructions[load_disp_index],
		(jive_output *[]){arg1}, (int64_t []){4})->outputs[0];

	jive_output * sum = jive_instruction_node_create(
		region,
		&jive_testarch_instructions[add_index],
		(jive_output *[]){v1, v2}, 0)->outputs[0];
	
	jive_subroutine_value_return(subroutine, 0, sum);
	
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
