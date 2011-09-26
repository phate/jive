#include <assert.h>
#include <locale.h>

#include <jive/vsdg.h>
#include <jive/view.h>
#include <jive/bitstring.h>


int main()
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	jive_node * b0 = jive_bitconstant_create(graph, 8, "00110011");
	jive_node * b1 = jive_bitconstant(graph, 8, "00110011")->node;
	jive_node * b2 = jive_bitconstant_create_unsigned(graph, 8, 204);
	jive_node * b3 = jive_bitconstant_signed(graph, 8, 204)->node;

	assert(b0 == b1);
	assert(b0 == b2);
	assert(b0 == b3);

	jive_view(graph, stdout);
	jive_graph_destroy(graph);	
	
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}
