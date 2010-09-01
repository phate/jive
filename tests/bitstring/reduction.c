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
	
	jive_graph_prune(graph);
	
	jive_bitstring * x = jive_bitsymbolicconstant_create(graph, 16, "x");
	jive_bitstring * y = jive_bitsymbolicconstant_create(graph, 16, "y");
	
	{
		jive_bitstring * concat = jive_bitconcat(2, (jive_bitstring * []){x, y});
		jive_bitstring * slice = jive_bitslice(concat, 8, 24);
		jive_node * node = ((jive_output *) slice)->node;
		assert(node->class_ == &JIVE_BITCONCAT_NODE);
		assert(node->ninputs == 2);
		assert(node->inputs[0]->origin->node->class_ == &JIVE_BITSLICE_NODE);
		assert(node->inputs[1]->origin->node->class_ == &JIVE_BITSLICE_NODE);
		
		const jive_bitslice_node_attrs * attrs;
		attrs = (const jive_bitslice_node_attrs *) jive_node_get_attrs(node->inputs[0]->origin->node);
		assert( (attrs->low == 8) && (attrs->high == 16) );
		attrs = (const jive_bitslice_node_attrs *) jive_node_get_attrs(node->inputs[1]->origin->node);
		assert( (attrs->low == 0) && (attrs->high == 8) );
		
		assert(node->inputs[0]->origin->node->inputs[0]->origin->node == x->base.base.node);
		assert(node->inputs[1]->origin->node->inputs[0]->origin->node == y->base.base.node);
	}
	
	{
		jive_bitstring * slice1 = jive_bitslice(x, 0, 8);
		jive_bitstring * slice2 = jive_bitslice(x, 8, 16);
		jive_bitstring * concat = jive_bitconcat(2, (jive_bitstring *[]){slice1, slice2});
		assert(concat == x);
	}
	
	jive_graph_destroy(graph);
	
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);
	
	return 0;
}
