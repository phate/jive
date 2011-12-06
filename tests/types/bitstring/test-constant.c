#include <assert.h>
#include <locale.h>

#include <jive/vsdg.h>
#include <jive/view.h>
#include <jive/types/bitstring.h>

#define ZERO_64 "00000000" "00000000" "00000000" "00000000" "00000000" "00000000" "00000000" "00000000"
#define ONE_64  "10000000" "00000000" "00000000" "00000000" "00000000" "00000000" "00000000" "00000000"
#define MONE_64 "11111111" "11111111" "11111111" "11111111" "11111111" "11111111" "11111111" "11111111"

int main()
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	jive_node * b0 = jive_bitconstant_create(graph, 8, "00110011");
	jive_node * b1 = jive_bitconstant(graph, 8, "00110011")->node;
	jive_node * b2 = jive_bitconstant_create_unsigned(graph, 8, 204);
	jive_node * b3 = jive_bitconstant_signed(graph, 8, 204)->node;
	jive_node * b4 = jive_bitconstant_create(graph, 9, "001100110");
	
	assert(jive_bitconstant_equals_unsigned((jive_bitconstant_node *) b0, 204));
	assert(jive_bitconstant_equals_signed((jive_bitconstant_node *) b0, -52));
	assert(!jive_bitconstant_equals_signed((jive_bitconstant_node *) b0, 204));

	assert(b0 == b1);
	assert(b0 == b2);
	assert(b0 == b3);
	
	assert(jive_bitconstant_node_to_unsigned(jive_bitconstant_node_cast(b0)) == 204);
	assert(jive_bitconstant_node_to_signed(jive_bitconstant_node_cast(b0)) == -52);
	assert(jive_bitconstant_node_to_unsigned(jive_bitconstant_node_cast(b4)) == 204);
	assert(jive_bitconstant_node_to_signed(jive_bitconstant_node_cast(b4)) == 204);
	
	jive_node * plus_one_128 = jive_bitconstant_create(graph, 128, ONE_64 ZERO_64);
	assert(jive_bitconstant_equals_unsigned((jive_bitconstant_node *) plus_one_128, 1));
	assert(jive_bitconstant_equals_signed((jive_bitconstant_node *) plus_one_128, 1));

	jive_node * minus_one_128 = jive_bitconstant_create(graph, 128, MONE_64 MONE_64);
	assert(!jive_bitconstant_equals_unsigned((jive_bitconstant_node *) minus_one_128, (uint64_t) -1LL));
	assert(jive_bitconstant_equals_signed((jive_bitconstant_node *) minus_one_128, (uint64_t) -1LL));

	jive_view(graph, stdout);
	jive_graph_destroy(graph);	
	
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}
