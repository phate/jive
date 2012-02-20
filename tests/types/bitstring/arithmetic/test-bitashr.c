#include <locale.h>
#include <assert.h>

#include <jive/vsdg.h>
#include <jive/view.h>
#include <jive/types/bitstring.h>
#include <jive/vsdg/node-private.h>

int main()
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	JIVE_DECLARE_BITSTRING_TYPE(bits32, 32);
	jive_node * top = jive_node_create(graph->root_region,
		0, NULL, NULL,
		2, (const jive_type *[]){bits32, bits32});
	jive_node_reserve(top);

	jive_output * c0 = jive_bitconstant_unsigned(graph, 32, 16);
	jive_output * c1 = jive_bitconstant_signed(graph, 32, -16);
	jive_output * c2 = jive_bitconstant_unsigned(graph, 32, 2);
	jive_output * c3 = jive_bitconstant_unsigned(graph, 32, 32);

	jive_output * ashr0 = jive_bitashr(top->outputs[0], top->outputs[1]);
	jive_output * ashr1 = jive_bitashr(c0, c2);
	jive_output * ashr2 = jive_bitashr(c0, c3);
	jive_output * ashr3 = jive_bitashr(c1, c2);
	jive_output * ashr4 = jive_bitashr(c1, c3);

	jive_node * bottom = jive_node_create(graph->root_region,
		5, (const jive_type *[]){bits32, bits32, bits32, bits32, bits32},
		(jive_output *[]){ashr0, ashr1, ashr2, ashr3, ashr4}, 0, NULL);
	jive_node_reserve(bottom);

	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(jive_node_isinstance(bottom->inputs[0]->origin->node, &JIVE_BITASHR_NODE));
	assert(jive_node_isinstance(bottom->inputs[1]->origin->node, &JIVE_BITCONSTANT_NODE));
	assert(jive_node_isinstance(bottom->inputs[2]->origin->node, &JIVE_BITCONSTANT_NODE));
	assert(jive_node_isinstance(bottom->inputs[3]->origin->node, &JIVE_BITCONSTANT_NODE));
	assert(jive_node_isinstance(bottom->inputs[4]->origin->node, &JIVE_BITCONSTANT_NODE));

	jive_bitconstant_node * bc1 = jive_bitconstant_node_cast(bottom->inputs[1]->origin->node);
	jive_bitconstant_node * bc2 = jive_bitconstant_node_cast(bottom->inputs[2]->origin->node);
	jive_bitconstant_node * bc3 = jive_bitconstant_node_cast(bottom->inputs[3]->origin->node);
	jive_bitconstant_node * bc4 = jive_bitconstant_node_cast(bottom->inputs[4]->origin->node);
	assert(jive_bitconstant_equals_unsigned(bc1, 4));
	assert(jive_bitconstant_equals_unsigned(bc2, 0));
	assert(jive_bitconstant_equals_signed(bc3, -4));
	assert(jive_bitconstant_equals_signed(bc4, -1));

	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}
