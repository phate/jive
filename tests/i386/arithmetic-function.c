#include <assert.h>

#include <jive/buffer.h>
#include <jive/internal/instruction_str.h>
#include <jive/i386/machine.h>
#include <jive/i386/abi.h>
#include <jive/passthrough.h>
#include <jive/regalloc.h>
#include <jive/graphdebug.h>
#include <jive/subroutine.h>

typedef long (*binary_function_t)(long, long);

binary_function_t
make_binary_function(jive_i386_instruction_index instruction_type)
{
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_subroutine * subroutine = jive_i386_subroutine_create(graph, 2, true);
	
	jive_value * v1 = jive_subroutine_parameter(subroutine, 0);
	jive_value * v2 = jive_subroutine_parameter(subroutine, 1);
	
	jive_node * sub = jive_instruction_create(graph,
		&jive_i386_instructions[instruction_type],
		(jive_value *[]){v1, v2}, 0);
	
	jive_value * value = (jive_value *) jive_instruction_output(sub, 0);
	
	jive_subroutine_return_value(subroutine, value);
	
	jive_regalloc(graph, &jive_i386_machine, 0);
	
	jive_instruction_sequence seq;
	jive_graph_sequentialize(graph, &seq);
	
	jive_buffer buffer;
	jive_buffer_init(&buffer);
	jive_instruction_sequence_encode(&seq, &buffer, &jive_i386_machine);
	
	binary_function_t function = (binary_function_t) jive_buffer_executable(&buffer);
	
	jive_buffer_destroy(&buffer);
	
	jive_context_destroy(ctx);
	
	return function;
}

static long c_add(long a, long b) {return a+b;}
static long c_sub(long a, long b) {return a-b;}
static long c_mul(long a, long b) {return a*b;}
static long c_and(long a, long b) {return a&b;}
static long c_or(long a, long b) {return a|b;}
static long c_xor(long a, long b) {return a^b;}

static void
verify_function(binary_function_t test, binary_function_t ref)
{
	long a, b;
	for(a = -128; a<=128; a++)
		for(b = -128; b<=128; b++)
			assert( test(a,b) == ref(a,b) );
}

int main()
{
	binary_function_t jive_add = make_binary_function(jive_i386_int_add);
	binary_function_t jive_sub = make_binary_function(jive_i386_int_sub);
	binary_function_t jive_mul = make_binary_function(jive_i386_int_mul);
	binary_function_t jive_and = make_binary_function(jive_i386_int_and);
	binary_function_t jive_or = make_binary_function(jive_i386_int_or);
	binary_function_t jive_xor = make_binary_function(jive_i386_int_xor);
	
	verify_function(jive_add, c_add);
	verify_function(jive_sub, c_sub);
	verify_function(jive_and, c_and);
	verify_function(jive_or, c_or);
	verify_function(jive_xor, c_xor);
	verify_function(jive_mul, c_mul);
	
	return 0;
}

