#include <assert.h>

#include <jive/passthrough.h>
#include <jive/graphdebug.h>
#include <jive/subroutine.h>
#include <jive/fixed.h>
#include <jive/target.h>

typedef jive_value * (*node_creator_t)(jive_value *, jive_value *);
typedef long (*binary_function_t)(long, long);

static jive_value *
wrap_fixedand_create(jive_value * a, jive_value * b)
{
	jive_value * values[2] = {a, b};
	return jive_bitand(2, values);
}

static jive_value *
wrap_fixedor_create(jive_value * a, jive_value * b)
{
	jive_value * values[2] = {a, b};
	return jive_bitor(2, values);
}

static jive_value *
wrap_fixedxor_create(jive_value * a, jive_value * b)
{
	jive_value * values[2] = {a, b};
	return jive_bitxor(2, values);
}

static jive_value *
wrap_fixedadd_create(jive_value * a, jive_value * b)
{
	jive_value * values[2] = {a, b};
	return jive_intsum(2, values);
}

static jive_value *
wrap_fixedmul_create(jive_value * a, jive_value * b)
{
	return jive_fixedmul_create(a, b, 32);
}

static jive_value *
wrap_fixedsub_create(jive_value * a, jive_value * b)
{
	jive_value * values[2] = {a, jive_intneg(b)};
	return jive_intsum(2, values);
}

binary_function_t
make_binary_function(node_creator_t create)
{
	const jive_target * target = jive_get_default_target();
	
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_argument_type arguments[] = {jive_argument_long, jive_argument_long};
	jive_subroutine * subroutine = target->create_subroutine(graph, arguments, 2, jive_argument_long);
	
	jive_value * v1 = jive_subroutine_parameter(subroutine, 0);
	jive_value * v2 = jive_subroutine_parameter(subroutine, 1);
	
	jive_value * value = create(v1, v2);
	
	jive_subroutine_return_value(subroutine, value);
	
	binary_function_t function = jive_target_compile_executable(target, graph);
	
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
	binary_function_t jive_add = make_binary_function(wrap_fixedadd_create);
	binary_function_t jive_sub = make_binary_function(wrap_fixedsub_create);
	binary_function_t jive_mul = make_binary_function(wrap_fixedmul_create);
	binary_function_t jive_and = make_binary_function(wrap_fixedand_create);
	binary_function_t jive_or = make_binary_function(wrap_fixedor_create);
	binary_function_t jive_xor = make_binary_function(wrap_fixedxor_create);
	
	verify_function(jive_add, c_add);
	verify_function(jive_sub, c_sub);
	verify_function(jive_and, c_and);
	verify_function(jive_or, c_or);
	verify_function(jive_xor, c_xor);
	verify_function(jive_mul, c_mul);
	
	return 0;
}
