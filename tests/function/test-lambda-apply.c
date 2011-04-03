#include <assert.h>
#include <locale.h>
#include <jive/bitstring.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/function.h>

int main()
{
	setlocale(LC_ALL, "");
	jive_context * ctx = jive_context_create();
	
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_region * function = jive_function_region_create(graph->root_region);
	
	JIVE_DECLARE_BITSTRING_TYPE(int32, 32);
	jive_gate * arg1_gate = jive_type_create_gate(int32, graph, "arg1");
	jive_gate * arg2_gate = jive_type_create_gate(int32, graph, "arg2");
	jive_gate * ret_gate = jive_type_create_gate(int32, graph, "ret");
	
	jive_output * arg1 = jive_node_gate_output(function->top, arg1_gate);
	jive_output * arg2 = jive_node_gate_output(function->top, arg2_gate);
	jive_output * sum = jive_bitsum(2, (jive_output *[]){arg1, arg2});
	jive_node_gate_input(function->bottom, ret_gate, sum);
	
	jive_output * lambda_expr = jive_lambda_create(function);
	
	jive_output * c0 = jive_bitconstant(graph, 32, "01010100000000000000000000000000");
	jive_output * c1 = jive_bitconstant(graph, 32, "10010010000000000000000000000000");
	
	jive_node * apply_node = jive_apply_node_create(graph->root_region,
		lambda_expr, 2, (jive_output *[]){c0, c1});
	
	assert(jive_type_equals(int32, jive_output_get_type(apply_node->outputs[0])));
	
	jive_view(graph, stderr);
	
	jive_graph_destroy(graph);
	
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	return 0;
}
