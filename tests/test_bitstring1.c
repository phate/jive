#include <assert.h>
#include <string.h>

#include <jive/graph.h>
#include <jive/bitstring.h>
#include <jive/graphdebug.h>

int main()
{
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_value * base_const1 = jive_bitconstant(graph, 8, "00110111");
	jive_value * base_const2 = jive_bitconstant(graph, 8, "11001000");

	jive_value * base_x = jive_bitsymbolicconstant(graph, "x", 8);
	jive_value * base_y = jive_bitsymbolicconstant(graph, "y", 8);
	jive_value * base_z = jive_bitsymbolicconstant(graph, "z", 8);
	
	{
		/* slice of constant */
		jive_value_bits * a = (jive_value_bits *) jive_bitslice(base_const1, 2, 6);
		
		assert(a->node->type == &JIVE_BITCONSTANT);
		assert(a->nbits==4);
		assert(memcmp(jive_bitconstant_info(a->node)->bits, "1101", 4)==0);
	}
	
	{
		/* slice of slice */
		jive_value * a = jive_bitslice(base_x, 2, 6);
		jive_value_bits * b = (jive_value_bits *) jive_bitslice(a, 1, 3);
	
		assert(b->node->type == &JIVE_BITSLICE);
		const jive_bitslice_nodedata * data = jive_bitslice_info(b->node);
		assert(data->low==3 && data->high==5);
	}
	
	{
		/* slice of full node */
		jive_value_bits * a = (jive_value_bits *) jive_bitslice(base_x, 0, 8);
		
		assert(a->node->type == &JIVE_BITSYMBOLICCONSTANT);
		assert(strcmp(jive_bitsymbolicconstant_name(a->node), "x")==0);
	}
	
	{
		/* slice of concat */
		jive_value * list1[] = {base_x, base_y};
		jive_value * a = jive_bitconcat(2, list1);
		jive_value * b = jive_bitslice(a, 0, 8);
		
		assert(b==base_x);
	}
	
	{
		/* concat flattening */
		jive_value * list1[] = {base_x, base_y};
		jive_value * a = jive_bitconcat(2, list1);
		
		jive_value * list2[] = {a, base_z};
		jive_value_bits * b = (jive_value_bits *) jive_bitconcat(2, list2);
		
		assert(b->node->type == &JIVE_BITCONCAT);
		assert(jive_bitstring_ninputs(b->node)==3);
		assert(jive_bitstring_input(b->node, 0) == base_x);
		assert(jive_bitstring_input(b->node, 1) == base_y);
		assert(jive_bitstring_input(b->node, 2) == base_z);
	}
	
	{
		/* concat of single node */
		jive_value * a = jive_bitconcat(1, &base_x);
		
		assert(a==base_x);
	}
	
	{
		/* concat of slices */
		jive_value * a = jive_bitslice(base_x, 0, 4);
		jive_value * b = jive_bitslice(base_x, 4, 8);
		jive_value * list1[] = {a, b};
		jive_value * c = jive_bitconcat(2, list1);
		
		assert(c==base_x);
	}
	
	{
		/* concat of constants */
		jive_value * list1[] = {base_const1, base_const2};
		jive_value * a = jive_bitconcat(2, list1);
		
		assert(a->node->type == &JIVE_BITCONSTANT);
		assert(jive_bitconstant_info(a->node)->nbits == 16);
		assert(memcmp(jive_bitconstant_info(a->node)->bits, "0011011111001000", 16)==0);
	}
	
	{
		/* CSE */
		jive_value * a = jive_bitsymbolicconstant(graph, "x", 8);
		assert(a == base_x);
		
		jive_value * b = jive_bitconstant(graph, 8, "00110111");
		assert(b == base_const1);
		
		jive_value * c = jive_bitslice(base_x, 2, 6);
		jive_value * d = jive_bitslice(base_x, 2, 6);
		assert(c == d);
		
		jive_value * list1[] = {base_x, base_y};
		jive_value * e = jive_bitconcat(2, list1);
		jive_value * f = jive_bitconcat(2, list1);
		assert(e == f);
	}
	
	//jive_graph_view(graph);
	
	jive_context_destroy(ctx);
	
	return 0;
}


