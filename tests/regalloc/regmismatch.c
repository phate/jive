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
	
	jive_stackframe * stackframe = jive_context_malloc(context, sizeof(*stackframe));
	stackframe->stackptr = (jive_value_output *)stackptr;
	(*graph)->root_region->stackframe = stackframe;
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

#if 0
def create_testgraph_mismatch3():
	graph = vsdg.Graph()
	
	enter = vsdg.instruction.InstructionNode(graph.root_region, nop_instr)
	leave = vsdg.instruction.InstructionNode(graph.root_region, nop_instr)
	stackptr_var = cls_r4.create_gate(graph, "stackptr")
	stackptr_var.may_spill = False
	stackptr = enter.gate_output(stackptr_var)
	leave.gate_input(stackptr_var, stackptr)
	stack = vsdg.stack.Stackframe(stackptr)
	
	arg1 = enter.gate_output(cls_r1.create_gate(graph, "cls1"))
	leave.gate_input(regs.create_gate(graph, "reg"), arg1)
	
	arg2 = enter.gate_output(regs.create_gate(graph, "reg"))
	arg2 = vsdg.instruction.InstructionNode(graph.root_region, set_r1_instr, (arg2,)).outputs[0]
	leave.gate_input(regs.create_gate(graph, "reg"), arg2)
	
	graph.root_region.stackframe = stack
	
	return graph

def create_testgraph_mismatch4():
	graph = vsdg.Graph()
	
	enter = vsdg.instruction.InstructionNode(graph.root_region, nop_instr)
	leave = vsdg.instruction.InstructionNode(graph.root_region, nop_instr)
	stackptr_var = cls_r4.create_gate(graph, "stackptr")
	stackptr_var.may_spill = False
	stackptr = enter.gate_output(stackptr_var)
	leave.gate_input(stackptr_var, stackptr)
	stack = vsdg.stack.Stackframe(stackptr)
	
	arg1 = enter.gate_output(regs.create_gate(graph, "reg"))
	leave.gate_input(cls_r1.create_gate(graph, "cls1"), arg1)
	
	arg2 = enter.gate_output(regs.create_gate(graph, "reg"))
	arg2 = vsdg.instruction.InstructionNode(graph.root_region, set_r1_instr, (arg2,)).outputs[0]
	leave.gate_input(regs.create_gate(graph, "reg"), arg2)
	
	graph.root_region.stackframe = stack
	
	return graph

def create_testgraph_mismatch5():
	graph = vsdg.Graph()
	
	enter = vsdg.instruction.InstructionNode(graph.root_region, nop_instr)
	leave = vsdg.instruction.InstructionNode(graph.root_region, nop_instr)
	stackptr_var = cls_r4.create_gate(graph, "stackptr")
	stackptr_var.may_spill = False
	stackptr = enter.gate_output(stackptr_var)
	leave.gate_input(stackptr_var, stackptr)
	stack = vsdg.stack.Stackframe(stackptr)
	graph.root_region.stackframe = stack
	
	arg1 = enter.gate_output(regs.create_gate(graph, "reg"))
	
	tmp1 = vsdg.instruction.InstructionNode(graph.root_region, nop_instr)
	tmp1.gate_input(cls_r1.create_gate(graph, "cls1"), arg1)
	out1 = tmp1.gate_output(regs.create_gate(graph, "regs"))
	
	tmp2 = vsdg.instruction.InstructionNode(graph.root_region, nop_instr)
	tmp2.gate_input(cls_r2.create_gate(graph, "cls2"), arg1)
	out2 = tmp2.gate_output(regs.create_gate(graph, "regs"))
	
	leave.gate_input(regs.create_gate(graph, "retval1"), out1)
	leave.gate_input(regs.create_gate(graph, "retval2"), out2)
	
	return graph

def create_testgraph_mismatch6():
	graph = vsdg.Graph()
	
	enter = vsdg.instruction.InstructionNode(graph.root_region, nop_instr)
	mid = vsdg.instruction.InstructionNode(graph.root_region, nop_instr)
	leave = vsdg.instruction.InstructionNode(graph.root_region, nop_instr)
	stackptr_var = cls_r4.create_gate(graph, "stackptr")
	stackptr_var.may_spill = False
	stackptr = enter.gate_output(stackptr_var)
	leave.gate_input(stackptr_var, stackptr)
	stack = vsdg.stack.Stackframe(stackptr)
	graph.root_region.stackframe = stack
	
	mid.gate_input(regs.create_gate(graph, "anon"), enter.gate_output(regs.create_gate(graph, "anon")))
	
	leave.gate_input(regs.create_gate(graph, "anon"), enter.gate_output(regs.create_gate(graph, "anon")))
	leave.gate_input(regs.create_gate(graph, "anon"), enter.gate_output(regs.create_gate(graph, "anon")))
	
	leave.gate_input(cls_r2.create_gate(graph, "r2"), mid.gate_output(regs.create_gate(graph, "anon")))
	mid.gate_output(cls_r2.create_gate(graph, "r2"))
	
	return graph
#endif





typedef jive_graph * (*creator_function_t)(jive_context *);

static const creator_function_t tests[] = {
	create_testgraph_mismatch1,
	create_testgraph_mismatch2,
	create_testgraph_mismatch3,
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
	
	//assert(jive_context_is_empty(context));
	jive_context_destroy(context);
	
	return 0;
}
