#include <assert.h>
#include <stdio.h>
#include <locale.h>

#include <jive/backend/i386/instructionset.h>
#include <jive/backend/i386/registerset.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/shaped-variable.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/util/buffer.h>

int main()
{
	setlocale(LC_ALL, "");
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_node * enter = jive_instruction_node_create(
		graph->root_region,
		&jive_i386_instructions[jive_i386_int_load_imm],
		0, (const long[]){42});
	
	jive_node * leave = jive_instruction_node_create(
		graph->root_region,
		&jive_i386_instructions[jive_i386_ret],
		0, NULL);
	
	const jive_type * type = jive_output_get_type(enter->outputs[0]);
	jive_gate * gate = jive_type_create_gate(type, graph, "retval");
	gate -> required_rescls = &jive_i386_regcls[jive_i386_gpr_eax].base;
	jive_node_gate_input(leave, gate, enter->outputs[0]);
	
	jive_view(graph, stderr);
	
	jive_shaped_graph * shaped_graph = jive_shaped_graph_create(graph);
	
	jive_ssavar * ssavar = jive_output_auto_merge_variable(enter->outputs[0]);
	
	jive_shaped_variable * regvar = jive_shaped_graph_map_variable(shaped_graph, ssavar->variable);
	
	const jive_resource_name * reg_eax = &jive_i386_regs[jive_i386_eax].base;
	assert(jive_shaped_variable_allowed_resource_name(regvar, reg_eax));
	assert(jive_shaped_variable_allowed_resource_name_count(regvar) == 1);
	
	jive_variable_set_resource_name(ssavar->variable, reg_eax);
	
	jive_view(graph, stderr);
	
	jive_buffer buffer;
	jive_buffer_init(&buffer, ctx);
	jive_graph_generate_code(graph, &buffer);
	int (*function)(void) = (int(*)(void)) jive_buffer_executable(&buffer);
	jive_buffer_fini(&buffer);
	
	jive_shaped_graph_destroy(shaped_graph);
	
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(ctx));
	
	jive_context_destroy(ctx);
	
	int ret_value = function();
	assert(ret_value == 42);
	
	return 0;
}

