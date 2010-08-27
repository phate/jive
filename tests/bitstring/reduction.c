#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>

#include <jive/vsdg.h>
#include <jive/view.h>
#include <jive/bitstring.h>

static void
assert_constant(jive_bitstring * bitstr, size_t nbits, const char bits[])
{
	jive_bitconstant_node * node = jive_bitconstant_node_cast(bitstr->base.base.node);
	assert(node);
	
	assert(node->attrs.nbits == nbits);
	assert(strncmp(node->attrs.bits, bits, nbits) == 0);
}

int main()
{
	setlocale(LC_ALL, "");
	
	jive_context * context = jive_context_create();
	
	jive_graph * graph = jive_graph_create(context);
	
	jive_bitstring * a = jive_bitconstant_create(graph, 4, "1100");
	jive_bitstring * b = jive_bitconstant_create(graph, 4, "1010");
	
	jive_bitstring * ops[] = {a, b};
	
	assert_constant(jive_bitand(2, ops), 4, "1000");
	assert_constant(jive_bitor(2, ops), 4, "1110");
	assert_constant(jive_bitxor(2, ops), 4, "0110");
	assert_constant(jive_bitadd(2, ops), 4, "0001");
	assert_constant(jive_bitmultiply(2, ops), 8, "11110000");
	assert_constant(jive_bitconcat(2, ops), 8, "11001010");
	
	jive_graph_destroy(graph);
	
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);
	
	return 0;
}
