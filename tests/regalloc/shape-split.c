#include <assert.h>
#include <locale.h>
#include <stdio.h>

#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc.h>
#include <jive/view.h>
#include <jive/vsdg.h>

#include "testarch.h"

jive_graph *
create_testgraph(jive_context * ctx)
{
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_node * enter = jive_instruction_node_create(
		graph->root_region,
		&jive_testarch_instructions[nop_index],
		0, 0);
	jive_node * leave = jive_instruction_node_create(
		graph->root_region,
		&jive_testarch_instructions[nop_index],
		0, 0);
	
	jive_gate * stackptr_gate = jive_register_class_create_gate(&jive_testarch_regcls[cls_r3], graph, "stackptr");
	stackptr_gate->may_spill = false;
	
	jive_output * stackptr = jive_node_gate_output(enter, stackptr_gate);
	jive_node_gate_input(leave, stackptr_gate, stackptr);
	
	jive_gate * callee_saved_r2 = jive_register_class_create_gate(&jive_testarch_regcls[cls_r2], graph, "saved_r2");
	jive_node_gate_input(leave, callee_saved_r2, jive_node_gate_output(enter, callee_saved_r2));
	
	jive_gate * callee_saved_r0 = jive_register_class_create_gate(&jive_testarch_regcls[cls_r0], graph, "saved_r0");
	jive_node_gate_input(leave, callee_saved_r0, jive_node_gate_output(enter, callee_saved_r0));
	
	jive_output * arg1 = jive_node_gate_output(enter, jive_register_class_create_gate(&jive_testarch_regcls[cls_r1], graph, "arg1"));
	
	jive_output * v1 = jive_instruction_node_create(
		graph->root_region,
		&jive_testarch_instructions[load_disp_index],
		(jive_output *[]){arg1}, (long []){0})->outputs[0];
	
	jive_output * v2 = jive_instruction_node_create(
		graph->root_region,
		&jive_testarch_instructions[load_disp_index],
		(jive_output *[]){arg1}, (long []){4})->outputs[0];

	jive_output * sum = jive_instruction_node_create(
		graph->root_region,
		&jive_testarch_instructions[add_index],
		(jive_output *[]){v1, v2}, 0)->outputs[0];
	
	jive_node_gate_input(leave, jive_register_class_create_gate(&jive_testarch_regcls[cls_r1], graph, "ret1"), sum);
	
	return graph;
}

int main()
{
	setlocale(LC_ALL, "");
	jive_context * ctx = jive_context_create();
	
	jive_graph * graph = create_testgraph(ctx);
	jive_view(graph, stdout);
	jive_shaped_graph * shaped_graph = jive_regalloc(graph, &jive_testarch_xfer_factory);
	jive_view(graph, stdout);
	jive_shaped_graph_destroy(shaped_graph);
	jive_graph_destroy(graph);
	
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	
	return 0;
}
