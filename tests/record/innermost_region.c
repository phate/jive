#include <assert.h>
#include <locale.h>
#include <jive/bitstring.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/function.h>
#include <jive/vsdg/record.h>
#include <jive/vsdg/recordlayout.h>

int main()
{
	setlocale(LC_ALL, "");

	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);

	jive_region * function = jive_function_region_create(graph->root_region);

	JIVE_DECLARE_BITSTRING_TYPE(int8, 8);
	jive_gate * arg_gate = jive_type_create_gate(int8, graph, "arg");
	jive_gate * ret_gate = jive_type_create_gate(int8, graph, "ret");

	jive_output * o0 = jive_bitconstant(graph, 8, "00000010");
	jive_output * o1 = jive_node_gate_output(function->top, arg_gate);

	const jive_record_layout_element l_elements[] = {
		{(jive_value_type*)int8,0},
		{(jive_value_type*)int8,1}
	};

	jive_record_layout * l = jive_record_layout_create(ctx, 2, l_elements, 4, 2);
	jive_output * g = jive_group_create(l, 2, (jive_output * []){o0, o1});
	jive_output * s = jive_select_create(1, g);

	jive_node_gate_input(function->bottom, ret_gate, s);
	jive_node * lambda_node = jive_lambda_node_create(function);
	jive_node_reserve(lambda_node);

	assert(function == g->node->region);

	jive_view(graph, stderr);

	jive_record_layout_destroy(l);

	jive_graph_destroy(graph);
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);

	return 0;	
}
