#include <assert.h>
#include <stdio.h>
#include <locale.h>

#include <jive/vsdg.h>
#include <jive/view.h>
#include <jive/util/buffer.h>
#include <jive/backend/i386/instructionset.h>
#include <jive/backend/i386/registerset.h>
#include <jive/backend/i386/machine.h>
#include <jive/backend/i386/stackframe.h>
#include <jive/backend/i386/subroutine.h>

#include <jive/regalloc.h>

typedef long (*dotprod_function_t)(const int *, const int *);

dotprod_function_t
make_dotprod_function(size_t vector_size)
{
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_subroutine * subroutine = jive_i386_subroutine_create(graph->root_region,
		2, (jive_argument_type []) { jive_argument_pointer, jive_argument_pointer },
		jive_argument_int);
	
	jive_output * p1 = jive_subroutine_get_parameter(subroutine, 0);
	jive_output * p2 = jive_subroutine_get_parameter(subroutine, 1);
	
	jive_output * operands[vector_size];
	size_t n;
	for(n=0; n<vector_size; n++) {
		long displacement = n * 4;
		jive_node * a1 = (jive_node *) jive_instruction_node_create(
			subroutine->region,
			&jive_i386_instructions[jive_i386_int_load32_disp],
			&p1, &displacement);
		jive_output * v1 = a1->outputs[0];
		jive_node * a2 = (jive_node *) jive_instruction_node_create(
			subroutine->region,
			&jive_i386_instructions[jive_i386_int_load32_disp],
			&p2, &displacement);
		jive_output * v2 = a2->outputs[0];
		
		jive_node * m = (jive_node *) jive_instruction_node_create(
			subroutine->region,
			&jive_i386_instructions[jive_i386_int_mul],
			(jive_output *[]){v1, v2}, 0);
		operands[n] = m->outputs[0];
	}
	
	jive_output * value = operands[0];
	for(n=1; n<vector_size; n++) {
		jive_node * s = (jive_node *) jive_instruction_node_create(
			subroutine->region,
			&jive_i386_instructions[jive_i386_int_add],
			(jive_output *[]){value, operands[n]}, 0);
		value = s->outputs[0];
	}
	
	jive_subroutine_return_value(subroutine, value);
	
	jive_regalloc(graph, &jive_i386_transfer_instructions_factory);
	
	jive_buffer buffer;
	jive_buffer_init(&buffer, ctx);
	jive_graph_generate_code(graph, &buffer);
	dotprod_function_t function = (dotprod_function_t) jive_buffer_executable(&buffer);
	
	jive_buffer_fini(&buffer);
	
	jive_context_destroy(ctx);
	
	return function;
}


int main(int argc, char ** argv)
{
	setlocale(LC_ALL, "");
	
	size_t count = 4, n;
	if (argc>=2) count = atoi(argv[1]);
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

