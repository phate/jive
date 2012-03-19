#include <assert.h>
#include <stdio.h>
#include <locale.h>

#include <jive/regalloc.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/view.h>
#include <jive/vsdg.h>

#include "testarch.h"

static jive_graph *
create_testgraph_mismatch1(jive_context * context)
{
	jive_graph * graph = jive_graph_create(context);
	
	jive_subroutine * subroutine = jive_testarch_subroutine_create(
		graph->root_region,
		2, (const jive_argument_type[]) { jive_argument_int, jive_argument_int },
		1, (const jive_argument_type[]) { jive_argument_int }
	);
	
	jive_output * arg2 = jive_subroutine_value_parameter(subroutine, 1);
	jive_subroutine_value_return(subroutine, 0, arg2);
	
	return graph;
}

static jive_graph *
create_testgraph_mismatch2(jive_context * context)
{
	jive_graph * graph = jive_graph_create(context);
	
	jive_subroutine * subroutine = jive_testarch_subroutine_create(
		graph->root_region,
		2, (const jive_argument_type[]) { jive_argument_int, jive_argument_int },
		2, (const jive_argument_type[]) { jive_argument_int, jive_argument_int }
	);
	
	jive_output * arg1 = jive_subroutine_value_parameter(subroutine, 0);
	jive_output * arg2 = jive_subroutine_value_parameter(subroutine, 1);
	jive_subroutine_value_return(subroutine, 0, arg2);
	jive_subroutine_value_return(subroutine, 1, arg1);
	
	return graph;
}

static jive_graph *
create_testgraph_mismatch3(jive_context * context)
{
	jive_graph * graph = jive_graph_create(context);
	jive_subroutine * subroutine = jive_testarch_subroutine_create(
		graph->root_region,
		0, NULL,
		0, NULL
	);
	jive_node * enter = &subroutine->enter->base;
	jive_node * leave = &subroutine->leave->base;
	
	jive_output * arg1 = jive_node_gate_output(enter, jive_register_class_create_gate(&jive_testarch_regcls[cls_r1], graph, "cls1"));
	jive_node_gate_input(leave, jive_register_class_create_gate(&jive_testarch_regcls[cls_gpr], graph, "reg"), arg1);
	
	jive_output * arg2 = jive_node_gate_output(enter, jive_register_class_create_gate(&jive_testarch_regcls[cls_gpr], graph, "reg"));
	jive_node * node = jive_instruction_node_create(
		subroutine->region,
		&jive_testarch_instructions[setr1_index],
		&arg2, NULL);
	arg2 = node->outputs[0];
	jive_node_gate_input(leave, jive_register_class_create_gate(&jive_testarch_regcls[cls_gpr], graph, "reg"), arg2);
	
	return graph;
}

static jive_graph *
create_testgraph_mismatch4(jive_context * context)
{
	jive_graph * graph = jive_graph_create(context);
	jive_subroutine * subroutine = jive_testarch_subroutine_create(
		graph->root_region,
		0, NULL,
		0, NULL
	);
	jive_node * enter = &subroutine->enter->base;
	jive_node * leave = &subroutine->leave->base;
	
	jive_output * arg1 = jive_node_gate_output(enter, jive_register_class_create_gate(&jive_testarch_regcls[cls_gpr], graph, "reg"));
	jive_node_gate_input(leave, jive_register_class_create_gate(&jive_testarch_regcls[cls_r1], graph, "cls1"), arg1);
	
	jive_output * arg2 = jive_node_gate_output(enter, jive_register_class_create_gate(&jive_testarch_regcls[cls_gpr], graph, "reg"));
	jive_node * node = jive_instruction_node_create(
		subroutine->region,
		&jive_testarch_instructions[setr1_index],
		&arg2, NULL);
	jive_node_gate_input(leave, jive_register_class_create_gate(&jive_testarch_regcls[cls_gpr], graph, "reg"), node->outputs[0]);
	
	return graph;
}

static jive_graph *
create_testgraph_mismatch5(jive_context * context)
{
	jive_graph * graph = jive_graph_create(context);
	jive_subroutine * subroutine = jive_testarch_subroutine_create(
		graph->root_region,
		0, NULL,
		0, NULL
	);
	jive_node * enter = &subroutine->enter->base;
	jive_node * leave = &subroutine->leave->base;
	
	jive_output * arg1 = jive_node_gate_output(enter, jive_register_class_create_gate(&jive_testarch_regcls[cls_gpr], graph, "reg"));
	
	jive_node * tmp1 = jive_instruction_node_create(
		subroutine->region,
		&JIVE_PSEUDO_NOP,
		NULL, NULL);
	jive_node_gate_input(tmp1, jive_register_class_create_gate(&jive_testarch_regcls[cls_r1], graph, "cls1"), arg1);
	jive_output * out1 = jive_node_gate_output(tmp1, jive_register_class_create_gate(&jive_testarch_regcls[cls_gpr], graph, "regs"));
	
	jive_node * tmp2 = jive_instruction_node_create(
		subroutine->region,
		&JIVE_PSEUDO_NOP,
		NULL, NULL);
	jive_node_gate_input(tmp2, jive_register_class_create_gate(&jive_testarch_regcls[cls_r2], graph, "cls2"), arg1);
	jive_output * out2 = jive_node_gate_output(tmp2, jive_register_class_create_gate(&jive_testarch_regcls[cls_gpr], graph, "regs"));
	
	jive_node_gate_input(leave, jive_register_class_create_gate(&jive_testarch_regcls[cls_gpr], graph, "retval1"), out1);
	jive_node_gate_input(leave, jive_register_class_create_gate(&jive_testarch_regcls[cls_gpr], graph, "retval2"), out2);
	
	return graph;
}

static jive_graph *
create_testgraph_mismatch6(jive_context * context)
{
	jive_graph * graph = jive_graph_create(context);
	jive_subroutine * subroutine = jive_testarch_subroutine_create(
		graph->root_region,
		0, NULL,
		0, NULL
	);
	jive_node * enter = &subroutine->enter->base;
	jive_node * leave = &subroutine->leave->base;
	
	jive_node * mid = jive_instruction_node_create(
		subroutine->region,
		&JIVE_PSEUDO_NOP,
		NULL, NULL);
	
	jive_node_gate_input(mid, jive_register_class_create_gate(&jive_testarch_regcls[cls_gpr], graph, "anon"), jive_node_gate_output(enter, jive_register_class_create_gate(&jive_testarch_regcls[cls_gpr], graph, "anon")));
	
	jive_node_gate_input(leave, jive_register_class_create_gate(&jive_testarch_regcls[cls_gpr], graph, "anon"), jive_node_gate_output(enter, jive_register_class_create_gate(&jive_testarch_regcls[cls_gpr], graph, "anon")));
	jive_node_gate_input(leave, jive_register_class_create_gate(&jive_testarch_regcls[cls_gpr], graph, "anon"), jive_node_gate_output(enter, jive_register_class_create_gate(&jive_testarch_regcls[cls_gpr], graph, "anon")));
	
	jive_node_gate_input(leave, jive_register_class_create_gate(&jive_testarch_regcls[cls_r2], graph, "r2"), jive_node_gate_output(mid, jive_register_class_create_gate(&jive_testarch_regcls[cls_gpr], graph, "anon")));
	jive_node_gate_output(mid, jive_register_class_create_gate(&jive_testarch_regcls[cls_r2], graph, "r2"));
	
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

int main()
{
	setlocale(LC_ALL, "");
	
	jive_context * context = jive_context_create();
	
	size_t n;
	for (n = 0; n < sizeof(tests)/sizeof(tests[0]); n++) {
		fprintf(stderr, "%zd\n", n);
		jive_graph * graph = tests[n](context);
		jive_view(graph, stdout);
		jive_shaped_graph * shaped_graph = jive_regalloc(graph, &jive_testarch_xfer_factory);
		jive_view(graph, stdout);
		jive_shaped_graph_destroy(shaped_graph);
		jive_graph_destroy(graph);
	}
	
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);
	
	return 0;
}
