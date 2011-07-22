#include <assert.h>
#include <stdio.h>
#include <locale.h>

#include <jive/arch/stackframe.h>
#include <jive/regalloc.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/view.h>
#include <jive/vsdg.h>


#include "testarch.h"

static void
proc_frame(jive_context * context, jive_graph ** graph, jive_node ** enter, jive_node ** leave)
{
	*graph = jive_graph_create(context);
	*enter = jive_instruction_node_create(
		(*graph)->root_region,
		&JIVE_PSEUDO_NOP,
		NULL, NULL);
	*leave = jive_instruction_node_create(
		(*graph)->root_region,
		&JIVE_PSEUDO_NOP,
		NULL, NULL);
	
	jive_gate * stackptr_var = jive_register_class_create_gate(&jive_testarch_regcls[cls_r0], *graph, "stackptr");
	stackptr_var->may_spill = false;
	jive_output * stackptr = jive_node_gate_output(*enter, stackptr_var);
	jive_node_gate_input(*leave, stackptr_var, stackptr);
}

static jive_graph *
create_testgraph_postop_xfer(jive_context * context)
{
	/* requires post-op transfer to satisfy register constraints */
	jive_graph * graph;
	jive_node * enter, * leave;
	proc_frame(context, &graph, &enter, &leave);
	
	jive_gate * arg1_gate = jive_register_class_create_gate(&jive_testarch_regcls[cls_r1], graph, "arg1");
	jive_gate * arg2_gate = jive_register_class_create_gate(&jive_testarch_regcls[cls_r2], graph, "arg2");
	jive_gate * retval_gate = jive_register_class_create_gate(&jive_testarch_regcls[cls_r3], graph, "retval");
	
	jive_output * arg1 = jive_node_gate_output(enter, arg1_gate);
	jive_output * arg2 = jive_node_gate_output(enter, arg2_gate);
	jive_output * retval = jive_instruction_node_create(
		graph->root_region,
		&jive_testarch_instructions[sub_gpr_index],
		(jive_output *[]) {arg1, arg2}, NULL)->outputs[0];
	jive_node_gate_input(leave, retval_gate, retval);
	
	return graph;
}

static jive_graph *
create_testgraph_preop_xfer(jive_context * context)
{
	/* requires pre-op transfer to satisfy register constraints */
	jive_graph * graph;
	jive_node * enter, * leave;
	proc_frame(context, &graph, &enter, &leave);
	
	jive_gate * arg1_gate = jive_register_class_create_gate(&jive_testarch_regcls[cls_r1], graph, "arg1");
	jive_gate * arg2_gate = jive_register_class_create_gate(&jive_testarch_regcls[cls_r2], graph, "arg2");
	jive_gate * retval_gate = jive_register_class_create_gate(&jive_testarch_regcls[cls_r3], graph, "retval");
	jive_gate * retval2_gate = jive_register_class_create_gate(&jive_testarch_regcls[cls_r1], graph, "retval2");
	
	jive_output * arg1 = jive_node_gate_output(enter, arg1_gate);
	jive_output * arg2 = jive_node_gate_output(enter, arg2_gate);
	jive_output * retval = jive_instruction_node_create(
		graph->root_region,
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
	jive_graph * graph;
	jive_node * enter, * leave;
	proc_frame(context, &graph, &enter, &leave);
	
	jive_gate * arg1_gate = jive_register_class_create_gate(&jive_testarch_regcls[cls_r1], graph, "arg1");
	jive_gate * arg2_gate = jive_register_class_create_gate(&jive_testarch_regcls[cls_r2], graph, "arg2");
	jive_gate * retval_gate = jive_register_class_create_gate(&jive_testarch_regcls[cls_r2], graph, "retval");
	jive_gate * retval2_gate = jive_register_class_create_gate(&jive_testarch_regcls[cls_r1], graph, "retval2");
	
	jive_output * arg1 = jive_node_gate_output(enter, arg1_gate);
	jive_output * arg2 = jive_node_gate_output(enter, arg2_gate);
	jive_output * retval = jive_instruction_node_create(
		graph->root_region,
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
	jive_graph * graph;
	jive_node * enter, * leave;
	proc_frame(context, &graph, &enter, &leave);
	
	jive_gate * arg1_gate = jive_register_class_create_gate(&jive_testarch_regcls[cls_r1], graph, "arg1");
	jive_gate * arg2_gate = jive_register_class_create_gate(&jive_testarch_regcls[cls_r2], graph, "arg2");
	jive_gate * save_r3_gate = jive_register_class_create_gate(&jive_testarch_regcls[cls_r3], graph, "arg2");
	jive_gate * retval_gate = jive_register_class_create_gate(&jive_testarch_regcls[cls_r2], graph, "retval");
	jive_gate * retval2_gate = jive_register_class_create_gate(&jive_testarch_regcls[cls_r1], graph, "retval2");
	
	jive_output * arg1 = jive_node_gate_output(enter, arg1_gate);
	jive_output * arg2 = jive_node_gate_output(enter, arg2_gate);
	jive_output * retval = jive_instruction_node_create(
		graph->root_region,
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
