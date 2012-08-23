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
create_testgraph_gatesplit(jive_context * context)
{
	jive_graph * graph = jive_graph_create(context);
	jive_subroutine * subroutine = jive_testarch_subroutine_create(
		graph->root_region,
		0, NULL,
		0, NULL
	);
	jive_node * enter = &subroutine->enter->base;
	jive_node * leave = &subroutine->leave->base;
	
	jive_gate * var1_gate = jive_register_class_create_gate(&jive_testarch_regcls[cls_gpr], graph, "var1");
	jive_gate * var2_gate = jive_register_class_create_gate(&jive_testarch_regcls[cls_gpr], graph, "var2");
	jive_gate * var3_gate = jive_register_class_create_gate(&jive_testarch_regcls[cls_gpr], graph, "var3");
	
	jive_node *n1 = jive_instruction_node_create(
		subroutine->region,
		&JIVE_PSEUDO_NOP,
		NULL, NULL);
	jive_node *n2 = jive_instruction_node_create(
		subroutine->region,
		&JIVE_PSEUDO_NOP,
		NULL, NULL);
	
	jive_node_gate_input(n1, var2_gate, jive_node_gate_output(enter, var1_gate));
	jive_node_gate_input(n2, var3_gate, jive_node_gate_output(n1, var2_gate));
	jive_node_gate_input(leave, var1_gate, jive_node_gate_output(n2, var3_gate));
	
	jive_gate * var_cls1 = jive_register_class_create_gate(&jive_testarch_regcls[cls_r1], graph, "cls1");
	jive_gate * var_cls2 = jive_register_class_create_gate(&jive_testarch_regcls[cls_r2], graph, "cls2");
	jive_gate * var_cls3 = jive_register_class_create_gate(&jive_testarch_regcls[cls_r3], graph, "cls3");
	
	jive_node_gate_output(enter, var_cls1);
	jive_node_gate_output(n1, var_cls2);
	jive_node_gate_output(n2, var_cls3);
	
	return graph;
}


typedef jive_graph * (*creator_function_t)(jive_context *);

static const creator_function_t tests[] = {
	create_testgraph_gatesplit
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

JIVE_UNIT_TEST_REGISTER("regalloc/test-gatesplit", test_main);
