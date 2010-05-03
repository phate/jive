#include <assert.h>
#include <string.h>

#include <jive/graph.h>
#include <jive/bitstring.h>
#include <jive/graphdebug.h>

#include <stdio.h>

static inline int min(int a, int b) {return a<b?a:b;}
static inline int max(int a, int b) {return a>b?a:b;}

int main()
{
	/* test value range propagation for bitstring manipulation node */
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_value * base_x = jive_bitsymbolicconstant(graph, "x", 4);
	jive_node * node_x = base_x->node;
	
	jive_value * ext1 = jive_extend_slice(base_x, -2, 6, true);
	assert(jive_value_nbits(ext1) == 8);
	jive_value * ext2 = jive_extend_slice(jive_extend_slice(base_x, 0, 5, true), -2, 6, true);
	assert(jive_value_nbits(ext1) == 8);
	jive_value * ext3 = jive_extend_slice(base_x, -2, 6, false);
	assert(jive_value_nbits(ext3) == 8);
	jive_value * ext4 = jive_extend_slice(base_x, -2, 3, true);
	assert(jive_value_nbits(ext4) == 5);
	
	int x;
	for(x=-8; x<8; x++) {
		jive_bitsymbolicconstant_set_value_range_numeric(node_x, x, x);
		
		const jive_bitstring_value_range * range;
		
		range = jive_value_bits_get_value_range(ext1);
		
		assert(range->low == x<<2);
		assert(range->high == x<<2);
		
		range = jive_value_bits_get_value_range(ext2);
		
		assert(range->low == x<<2);
		assert(range->high == x<<2);
		
		int x_unsigned = (x & 15);
		
		range = jive_value_bits_get_value_range(ext3);
		
		assert(range->low == x_unsigned<<2);
		assert(range->high == x_unsigned<<2);
		
		int y = (x<<29)>>29;
		
		range = jive_value_bits_get_value_range(ext4);
		
		assert(range->low == y<<2);
		assert(range->high == y<<2);
		
	}
	
	return 0;
}


