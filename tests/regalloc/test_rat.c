#include <jive/buffer.h>
#include <jive/internal/instruction_str.h>
#include <jive/regalloc.h>
#include <jive/graphdebug.h>
#include <jive/subroutine.h>

#include "rat.h"

int main(int argc, char ** argv)
{
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	jive_stackframe * frame = jive_default_stackframe_create(graph);
	
	jive_node * top = jive_instruction_create(graph,
		&jive_RAT_instructions[jive_RAT_produce],
		0, 0);
	
	jive_value * value = jive_instruction_output(top, 0);
	
	jive_node * bottom = jive_instruction_create(graph,
		&jive_RAT_instructions[jive_RAT_consume],
		&value, 0);
	
	jive_node * c1 = jive_instruction_create(graph,
		&jive_RAT_instructions[jive_RAT_combine_f0],
		(jive_value *[]){value, value}, 0);
	value = jive_instruction_output(c1, 0);
	
	jive_node * c2 = jive_instruction_create(graph,
		&jive_RAT_instructions[jive_RAT_combine_f1],
		(jive_value *[]){value, value}, 0);
	value = jive_instruction_output(c2, 0);
	
	jive_node * c3 = jive_instruction_create(graph,
		&jive_RAT_instructions[jive_RAT_consume_f1],
		&value, 0);
	
	jive_state_edge_create((jive_node *)c3, (jive_node *)bottom);
	
	jive_node_reserve((jive_node *)top);
	jive_node_reserve((jive_node *)bottom);
	
	if (argc>1) jive_graph_view(graph);
	jive_regalloc(graph, &jive_RAT_machine, frame);
	if (argc>1) jive_graph_view(graph);
	
	return 0;
}
