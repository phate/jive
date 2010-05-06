#include <jive/target.h>

#include <jive/buffer.h>
#include <jive/internal/instruction_str.h>
#include <jive/regalloc.h>
#include <jive/arithmetic-select.h>

void *
jive_target_compile_executable(const jive_target * target, jive_graph * graph)
{
	jive_arithmetic_select(graph, target->machine);
	target->machine->match_instructions(target->machine, graph);
	jive_graph_prune(graph);
	
	jive_regalloc(graph, target->machine, 0);
	
	jive_instruction_sequence seq;
	jive_graph_sequentialize(graph, &seq);
	
	jive_buffer buffer;
	jive_buffer_init(&buffer);
	jive_instruction_sequence_encode(&seq, &buffer, target->machine);
	
	return jive_buffer_executable(&buffer);
}

