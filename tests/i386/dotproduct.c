#include <assert.h>
#include <stdlib.h>

#include <jive/buffer.h>
#include <jive/internal/instruction_str.h>
#include <jive/i386/machine.h>
#include <jive/i386/abi.h>
#include <jive/passthrough.h>
#include <jive/regalloc.h>
#include <jive/graphdebug.h>
#include <jive/subroutine.h>

typedef long (*dotprod_function_t)(const int *, const int *);

dotprod_function_t
make_dotprod_function(size_t vector_size)
{
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_subroutine * subroutine = jive_i386_subroutine_create(graph, 2, true);
	
	jive_value * p1 = jive_subroutine_parameter(subroutine, 0);
	jive_value * p2 = jive_subroutine_parameter(subroutine, 1);
	
	jive_value * operands[vector_size];
	size_t n;
	for(n=0; n<vector_size; n++) {
		long displacement = n * 4;
		jive_node * a1 = jive_instruction_create(graph,
			&jive_i386_instructions[jive_i386_int_load32_disp],
			&p1, &displacement);
		jive_value * v1 = jive_instruction_output(a1, 0);
		jive_node * a2 = jive_instruction_create(graph,
			&jive_i386_instructions[jive_i386_int_load32_disp],
			&p2, &displacement);
		jive_value * v2 = jive_instruction_output(a2, 0);
		
		jive_node * m = jive_instruction_create(graph,
			&jive_i386_instructions[jive_i386_int_mul],
			(jive_value *[]){v1, v2}, 0);
		operands[n] = jive_instruction_output(m, 0);
	}
	
	jive_value * value = operands[0];
	for(n=1; n<vector_size; n++) {
		jive_node * s = jive_instruction_create(graph,
			&jive_i386_instructions[jive_i386_int_add],
			(jive_value *[]){operands[n], value}, 0);
		value = jive_instruction_output(s, 0);
	}
	
	jive_subroutine_return_value(subroutine, value);
	
	jive_stackframe * stackframe = jive_subroutine_stackframe(subroutine);
	jive_regalloc(graph, &jive_i386_machine, stackframe);
	
	jive_instruction_sequence seq;
	jive_graph_sequentialize(graph, &seq);
	jive_stackframe_finalize(stackframe);
	
	jive_buffer buffer;
	jive_buffer_init(&buffer);
	jive_instruction_sequence_encode(&seq, &buffer, &jive_i386_machine);
	
	dotprod_function_t function = (dotprod_function_t) jive_buffer_executable(&buffer);
	
	jive_buffer_destroy(&buffer);
	
	jive_context_destroy(ctx);
	
	return function;
}


int main(int argc, char ** argv)
{
	size_t count = 4, n;
	if (argc>2) count = atoi(argv[1]);
	if (count<4) count = 4;
	dotprod_function_t dotprod2 = make_dotprod_function(count);
	
	int a[count], b[count];
	for(n=0; n<count; n++) a[n] = b[n] = 0;
	a[0] = 1; a[1] = 4; a[2] = 0; a[3] = -3;
	b[0] = 3; b[1] = 7; b[2] = 0; b[3] = 5;
	int result = dotprod2(a, b);
	assert(result == 1*3 + 4*7 + -3*5 );
	
	return 0;
}

