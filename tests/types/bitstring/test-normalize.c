#include <locale.h>
#include <assert.h>

#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/function.h>
#include <jive/types/bitstring.h>

int main()
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	jive_region * fr = jive_function_region_create(graph->root_region);
	jive_node * top = jive_region_get_top_node(fr);
	jive_node * bottom = jive_region_get_bottom_node(fr);

	JIVE_DECLARE_BITSTRING_TYPE(bits32, 32);
	jive_gate * arg_gate = jive_type_create_gate(bits32, graph, "arg");
	jive_gate * ret_gate = jive_type_create_gate(bits32, graph, "ret");
	jive_output * arg = jive_node_gate_output(top, arg_gate);
	
	jive_output * c0 = jive_bitconstant_unsigned(graph, 32, 3);
	jive_output * c1 = jive_bitconstant_unsigned(graph, 32, 4);
	
	jive_node_normal_form * sum_nf = jive_graph_get_nodeclass_form(graph, &JIVE_BITSUM_NODE);
	assert(sum_nf);
	jive_node_normal_form_set_mutable(sum_nf, false);

	jive_output * sum0 = jive_bitsum(2, (jive_output *[]){arg, c0});
	assert(jive_node_isinstance(sum0->node, &JIVE_BITSUM_NODE));
	assert(sum0->node->noperands == 2);
	
	jive_output * sum1 = jive_bitsum(2, (jive_output *[]){sum0, c1});
	assert(jive_node_isinstance(sum1->node, &JIVE_BITSUM_NODE));
	assert(sum1->node->noperands == 2);
	
	jive_input * retval = jive_node_gate_input(bottom, ret_gate, sum1);

	jive_node * lambda_node = jive_lambda_node_create(fr);
	jive_node_reserve(lambda_node);
	
	jive_node_normal_form_set_mutable(sum_nf, true);
	jive_graph_normalize(graph);
	jive_graph_prune(graph);
	
	jive_output * expected_sum = retval->origin;
	assert(jive_node_isinstance(expected_sum->node, &JIVE_BITSUM_NODE));
	assert(expected_sum->node->noperands == 2);
	jive_output * op1 = expected_sum->node->inputs[0]->origin;
	jive_output * op2 = expected_sum->node->inputs[1]->origin;
	if (!jive_node_isinstance(op1->node, &JIVE_BITCONSTANT_NODE)) {
		jive_output * tmp = op1; op1 = op2; op2 = tmp;
	}
	assert(jive_node_isinstance(op1->node, &JIVE_BITCONSTANT_NODE));
	assert(jive_bitconstant_equals_unsigned((jive_bitconstant_node *) op1->node, 3+4));
	assert(op2 == arg);

	jive_view(graph, stdout);

	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}
