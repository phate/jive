#include <assert.h>
#include <stdio.h>
#include <locale.h>

#include <jive/vsdg.h>
#include <jive/view.h>

#include <jive/regalloc.h>
#include <jive/arch/stackframe.h>
#include "testarch.h"

static void
proc_frame(jive_context * context, jive_graph ** graph, jive_node ** enter, jive_node ** leave)
{
	*graph = jive_graph_create(context);
	*enter = (jive_node *) jive_instruction_node_create(
		(*graph)->root_region,
		&JIVE_PSEUDO_NOP,
		NULL, NULL);
	*leave = (jive_node *) jive_instruction_node_create(
		(*graph)->root_region,
		&JIVE_PSEUDO_NOP,
		NULL, NULL);
	
	jive_gate * stackptr_var = jive_regcls_create_gate(&jive_testarch_regcls[cls_r4], *graph, "stackptr");
	jive_output * stackptr = jive_node_gate_output(*enter, stackptr_var);
	jive_node_gate_input(*leave, stackptr_var, stackptr);
	
	jive_testarch_stackframe_create((*graph)->root_region, stackptr);
}

static jive_graph *
create_testgraph_mismatch1(jive_context * context)
{
	jive_graph * graph;
	jive_node * enter, * leave;
	proc_frame(context, &graph, &enter, &leave);
	
	jive_output * arg1 = jive_node_gate_output(enter, jive_regcls_create_gate(&jive_testarch_regcls[cls_r1], graph, "cls1"));
	jive_node_gate_input(leave, jive_regcls_create_gate(&jive_testarch_regcls[cls_r2], graph, "cls2"), arg1);
	
	return graph;
}

static jive_graph *
create_testgraph_mismatch2(jive_context * context)
{
	jive_graph * graph;
	jive_node * enter, * leave;
	proc_frame(context, &graph, &enter, &leave);
	
	jive_output * arg1 = jive_node_gate_output(enter, jive_regcls_create_gate(&jive_testarch_regcls[cls_r1], graph, "cls1"));
	jive_node_gate_input(leave, jive_regcls_create_gate(&jive_testarch_regcls[cls_r2], graph, "cls2"), arg1);
	
	jive_output * arg2 = jive_node_gate_output(enter, jive_regcls_create_gate(&jive_testarch_regcls[cls_r2], graph, "cls2"));
	jive_node_gate_input(leave, jive_regcls_create_gate(&jive_testarch_regcls[cls_r1], graph, "cls1"), arg2);
	
	return graph;
}

static jive_graph *
create_testgraph_mismatch3(jive_context * context)
{
	jive_graph * graph;
	jive_node * enter, * leave;
	proc_frame(context, &graph, &enter, &leave);
	
	jive_output * arg1 = jive_node_gate_output(enter, jive_regcls_create_gate(&jive_testarch_regcls[cls_r1], graph, "cls1"));
	jive_node_gate_input(leave, jive_regcls_create_gate(&jive_testarch_regcls[cls_regs], graph, "reg"), arg1);
	
	jive_output * arg2 = jive_node_gate_output(enter, jive_regcls_create_gate(&jive_testarch_regcls[cls_regs], graph, "reg"));
	jive_node * node = (jive_node *) jive_instruction_node_create(
		graph->root_region,
		&jive_testarch_instructions[instr_setr1],
		&arg2, NULL);
	arg2 = node->outputs[0];
	jive_node_gate_input(leave, jive_regcls_create_gate(&jive_testarch_regcls[cls_regs], graph, "reg"), arg2);
	
	return graph;
}

static jive_graph *
create_testgraph_mismatch4(jive_context * context)
{
	jive_graph * graph;
	jive_node * enter, * leave;
	proc_frame(context, &graph, &enter, &leave);
	
	jive_output * arg1 = jive_node_gate_output(enter, jive_regcls_create_gate(&jive_testarch_regcls[cls_regs], graph, "reg"));
	jive_node_gate_input(leave, jive_regcls_create_gate(&jive_testarch_regcls[cls_r1], graph, "cls1"), arg1);
	
	jive_output * arg2 = jive_node_gate_output(enter, jive_regcls_create_gate(&jive_testarch_regcls[cls_regs], graph, "reg"));
	jive_node * node = (jive_node *) jive_instruction_node_create(
		graph->root_region,
		&jive_testarch_instructions[instr_setr1],
		&arg2, NULL);
	jive_node_gate_input(leave, jive_regcls_create_gate(&jive_testarch_regcls[cls_regs], graph, "reg"), node->outputs[0]);
	
	return graph;
}

static jive_graph *
create_testgraph_mismatch5(jive_context * context)
{
	jive_graph * graph;
	jive_node * enter, * leave;
	proc_frame(context, &graph, &enter, &leave);
	
	jive_output * arg1 = jive_node_gate_output(enter, jive_regcls_create_gate(&jive_testarch_regcls[cls_regs], graph, "reg"));
	
	jive_node * tmp1 = (jive_node *) jive_instruction_node_create(
		graph->root_region,
		&JIVE_PSEUDO_NOP,
		NULL, NULL);
	jive_node_gate_input(tmp1, jive_regcls_create_gate(&jive_testarch_regcls[cls_r1], graph, "cls1"), arg1);
	jive_output * out1 = jive_node_gate_output(tmp1, jive_regcls_create_gate(&jive_testarch_regcls[cls_regs], graph, "regs"));
	
	jive_node * tmp2 = (jive_node *) jive_instruction_node_create(
		graph->root_region,
		&JIVE_PSEUDO_NOP,
		NULL, NULL);
	jive_node_gate_input(tmp2, jive_regcls_create_gate(&jive_testarch_regcls[cls_r2], graph, "cls2"), arg1);
	jive_output * out2 = jive_node_gate_output(tmp2, jive_regcls_create_gate(&jive_testarch_regcls[cls_regs], graph, "regs"));
	
	jive_node_gate_input(leave, jive_regcls_create_gate(&jive_testarch_regcls[cls_regs], graph, "retval1"), out1);
	jive_node_gate_input(leave, jive_regcls_create_gate(&jive_testarch_regcls[cls_regs], graph, "retval2"), out2);
	
	return graph;
}

static jive_graph *
create_testgraph_mismatch6(jive_context * context)
{
	jive_graph * graph;
	jive_node * enter, * leave;
	proc_frame(context, &graph, &enter, &leave);
	
	jive_node * mid = (jive_node *) jive_instruction_node_create(
		graph->root_region,
		&JIVE_PSEUDO_NOP,
		NULL, NULL);
	
	jive_node_gate_input(mid, jive_regcls_create_gate(&jive_testarch_regcls[cls_regs], graph, "anon"), jive_node_gate_output(enter, jive_regcls_create_gate(&jive_testarch_regcls[cls_regs], graph, "anon")));
	
	jive_node_gate_input(leave, jive_regcls_create_gate(&jive_testarch_regcls[cls_regs], graph, "anon"), jive_node_gate_output(enter, jive_regcls_create_gate(&jive_testarch_regcls[cls_regs], graph, "anon")));
	jive_node_gate_input(leave, jive_regcls_create_gate(&jive_testarch_regcls[cls_regs], graph, "anon"), jive_node_gate_output(enter, jive_regcls_create_gate(&jive_testarch_regcls[cls_regs], graph, "anon")));
	
	jive_node_gate_input(leave, jive_regcls_create_gate(&jive_testarch_regcls[cls_r2], graph, "r2"), jive_node_gate_output(mid, jive_regcls_create_gate(&jive_testarch_regcls[cls_regs], graph, "anon")));
	jive_node_gate_output(mid, jive_regcls_create_gate(&jive_testarch_regcls[cls_r2], graph, "r2"));
	
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
	for(n=0; n<sizeof(tests)/sizeof(tests[0]); n++) {
		jive_graph * graph = tests[n](context);
		jive_view(graph, stdout);
		jive_regalloc(graph, &testarch_transfer_instructions_factory);
		jive_view(graph, stdout);
		jive_graph_destroy(graph);
	}
	
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);
	
	return 0;
}
