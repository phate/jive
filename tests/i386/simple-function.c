#include <assert.h>
#include <stdio.h>
#include <locale.h>

#include <jive/vsdg.h>
#include <jive/view.h>
#include <jive/util/buffer.h>
#include <jive/backend/i386/instructionset.h>
#include <jive/backend/i386/registerset.h>

int main()
{
	setlocale(LC_ALL, "");
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_node * enter = (jive_node *) jive_instruction_node_create(
		graph->root_region,
		&jive_i386_instructions[jive_i386_int_load_imm],
		0, (const long[]){42});
	
	jive_node * leave = (jive_node *) jive_instruction_node_create(
		graph->root_region,
		&jive_i386_instructions[jive_i386_ret],
		0, NULL);
	
	const jive_type * type = jive_output_get_type(enter->outputs[0]);
	jive_gate * gate = jive_type_create_gate(type, graph, "retval");
	((jive_value_gate *) gate) -> required_regcls = &jive_i386_regcls[jive_i386_gpr_eax];
	jive_node_gate_input(leave, gate, enter->outputs[0]);
	
	jive_view(graph, stderr);
	
	jive_resource * resource = jive_type_create_resource(type, graph);
	jive_resource_assign_output(resource, enter->outputs[0]);
	jive_resource_assign_input(resource, leave->inputs[0]);
	jive_value_resource_set_cpureg((jive_value_resource *) resource,
		&jive_i386_regs[jive_i386_eax]);
	
	jive_view(graph, stderr);
	
	jive_buffer buffer;
	jive_buffer_init(&buffer, ctx);
	jive_graph_generate_code(graph, &buffer);
	int (*function)(void) = (int(*)(void)) jive_buffer_executable(&buffer);
	jive_buffer_fini(&buffer);
	
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(ctx));
	
	jive_context_destroy(ctx);
	
	int ret_value = function();
	assert(ret_value == 42);
	
	return 0;
}

