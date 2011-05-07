#include <assert.h>
#include <stdio.h>
#include <locale.h>

#include <jive/vsdg.h>
#include <jive/view.h>
#include <jive/util/buffer.h>
#include <jive/arch/transfer-instructions.h>
#include <jive/backend/i386/instructionset.h>
#include <jive/backend/i386/registerset.h>
#include <jive/backend/i386/machine.h>
#include <jive/backend/i386/classifier.h>
#include <jive/backend/i386/instrmatch.h>
#include <jive/bitstring/arithmetic.h>

#include <jive/regalloc.h>
#include <jive/regalloc/shaped-graph.h>

static jive_xfer_block
i386_create_xfer(jive_region * region, jive_output * origin,
	const jive_resource_class * in_class, const jive_resource_class * out_class)
{
	jive_xfer_block xfer;
	
	xfer.node = jive_instruction_node_create(
		region,
		&jive_i386_instructions[jive_i386_int_transfer],
		(jive_output *[]){origin}, NULL);
	xfer.input = xfer.node->inputs[0];
	xfer.output = xfer.node->outputs[0];
	
	return xfer;
}

const jive_transfer_instructions_factory i386_xfer_factory = {
	i386_create_xfer
};

int main()
{
	setlocale(LC_ALL, "");
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_node * enter = jive_instruction_node_create(
		graph->root_region,
		&JIVE_PSEUDO_NOP,
		NULL, NULL);
	
	jive_node * leave = jive_instruction_node_create(
		graph->root_region,
		&jive_i386_instructions[jive_i386_ret],
		NULL, NULL);
	
	jive_node_reserve(leave);
	
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
	
	jive_node * load_a = jive_instruction_node_create(
		graph->root_region,
		&jive_i386_instructions[jive_i386_int_load32_disp],
		(jive_output *[]){stackptr}, (long[]){4});
	
	jive_node * load_b = jive_instruction_node_create(
		graph->root_region,
		&jive_i386_instructions[jive_i386_int_load32_disp],
		(jive_output *[]){stackptr}, (long[]){8});
	
	jive_output * sum = jive_bitadd(2, (jive_output *[]){load_a->outputs[0], load_b->outputs[0]});
	
	jive_node_gate_input(leave, retval_var, sum);
	
	jive_view(graph, stdout);
	
	jive_regselector regselector;
	jive_regselector_init(&regselector, graph, &jive_i386_reg_classifier);
	jive_regselector_process(&regselector);
	jive_i386_match_instructions(graph, &regselector);
	jive_regselector_fini(&regselector);
	
	jive_graph_prune(graph);
	jive_view(graph, stdout);
	
	jive_shaped_graph * shaped_graph = jive_regalloc(graph, &i386_xfer_factory);
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

