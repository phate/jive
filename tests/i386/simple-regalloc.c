#include <assert.h>
#include <stdio.h>
#include <locale.h>

#include <jive/vsdg.h>
#include <jive/view.h>
#include <jive/util/buffer.h>
#include <jive/backend/i386/instructionset.h>
#include <jive/backend/i386/registerset.h>
#include <jive/backend/i386/machine.h>

#include <jive/regalloc.h>
#include <jive/regalloc/shaped-graph.h>

int main()
{
	setlocale(LC_ALL, "");
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_node * enter = (jive_node *) jive_instruction_node_create(
		graph->root_region,
		&JIVE_PSEUDO_NOP,
		NULL, NULL);
	
	jive_node * leave = (jive_node *) jive_instruction_node_create(
		graph->root_region,
		&jive_i386_instructions[jive_i386_ret],
		NULL, NULL);
	
	jive_gate * stackptr_var = jive_register_class_create_gate(
		&jive_i386_regcls[jive_i386_gpr_esp], graph, "stackptr");
	jive_gate * retval_var = jive_register_class_create_gate(
		&jive_i386_regcls[jive_i386_gpr_eax], graph, "retval");
	
	jive_gate * save_ebx = jive_register_class_create_gate(
		&jive_i386_regcls[jive_i386_gpr_ebx], graph, "save_ebx");
	jive_gate * save_ebp = jive_register_class_create_gate(
		&jive_i386_regcls[jive_i386_gpr_ebp], graph, "save_ebp");
	jive_gate * save_edi = jive_register_class_create_gate(
		&jive_i386_regcls[jive_i386_gpr_edi], graph, "save_edi");
	jive_gate * save_esi = jive_register_class_create_gate(
		&jive_i386_regcls[jive_i386_gpr_esi], graph, "save_esi");
	
	jive_output * stackptr = jive_node_gate_output(enter, stackptr_var);
	jive_node_gate_input(leave, stackptr_var, stackptr);
	jive_node_gate_input(leave, save_ebx, jive_node_gate_output(enter, save_ebx));
	jive_node_gate_input(leave, save_ebp, jive_node_gate_output(enter, save_ebp));
	jive_node_gate_input(leave, save_esi, jive_node_gate_output(enter, save_esi));
	jive_node_gate_input(leave, save_edi, jive_node_gate_output(enter, save_edi));
	
	jive_node * load_a = (jive_node *) jive_instruction_node_create(
		graph->root_region,
		&jive_i386_instructions[jive_i386_int_load32_disp],
		(jive_output *[]){stackptr}, (long[]){4});
	
	jive_node * load_b = (jive_node *) jive_instruction_node_create(
		graph->root_region,
		&jive_i386_instructions[jive_i386_int_load32_disp],
		(jive_output *[]){stackptr}, (long[]){8});
	
	jive_node * add = (jive_node *) jive_instruction_node_create(
		graph->root_region,
		&jive_i386_instructions[jive_i386_int_add],
		(jive_output *[]){load_a->outputs[0], load_b->outputs[0]}, NULL);
	
	jive_node_gate_input(leave, retval_var, add->outputs[0]);
	
	jive_view(graph, stdout);
	
	jive_shaped_graph * shaped_graph = jive_regalloc(graph, NULL);
	jive_shaped_graph_destroy(shaped_graph);
	
	jive_view(graph, stdout);
	
	jive_buffer buffer;
	jive_buffer_init(&buffer, ctx);
	jive_graph_generate_code(graph, &buffer);
	int (*function)(int, int) = (int(*)(int, int)) jive_buffer_executable(&buffer);
	jive_buffer_fini(&buffer);
	
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(ctx));
	
	jive_context_destroy(ctx);
	
	int ret_value = function(18, 24);
	assert(ret_value == 42);
	
	return 0;
}

