#include <assert.h>

#include <jive/buffer.h>
#include <jive/internal/instruction_str.h>
#include <jive/i386/machine.h>
#include <jive/i386/abi.h>
#include <jive/passthrough.h>
#include <jive/regalloc.h>
#include <jive/graphdebug.h>
#include <jive/subroutine.h>

int main()
{
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_argument_type arguments[] = {jive_argument_int};
	jive_subroutine * sub = jive_i386_subroutine_create(graph, arguments, 1, jive_argument_int);
	
	jive_value * value = jive_subroutine_parameter(sub, 0);
	jive_subroutine_return_value(sub, value);
	
	jive_regalloc(graph, &jive_i386_machine, 0);
	
	jive_instruction_sequence seq;
	jive_graph_sequentialize(graph, &seq);
	
	jive_buffer buffer;
	jive_buffer_init(&buffer);
	jive_instruction_sequence_encode(&seq, &buffer, &jive_i386_machine);
	
	int (*function)(int) = (int(*)(int)) jive_buffer_executable(&buffer);
	
	jive_buffer_destroy(&buffer);
	
	jive_context_destroy(ctx);
	
	int ret_value = function(42);
	assert(ret_value == 42);
	
	return 0;
}

