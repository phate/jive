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
	jive_value * base_y = jive_bitsymbolicconstant(graph, "y", 4);
	jive_node * node_y = base_y->node;
	
	jive_value * upperbit_x = jive_bitslice(base_x, 3, 4);
	jive_value * ext_x = jive_bitconcat(2, (jive_value *[]){base_x, upperbit_x});
	jive_value * upperbit_y = jive_bitslice(base_y, 3, 4);
	jive_value * ext_y = jive_bitconcat(2, (jive_value *[]){base_y, upperbit_y});
	
	int x_low, x_high, y_low, y_high;
	
	jive_value * neg = jive_intneg(ext_x);
	jive_value * ext_inputs[] = {ext_x, ext_y};
	jive_value * sum = jive_intsum(2, ext_inputs);
	jive_value * slice = jive_bitslice(base_x, 1, 4);
	jive_value * inputs[] = {base_x, base_y};
	jive_value * product = jive_intproduct(2, inputs);
	jive_value * bitand = jive_bitand(2, inputs);
	jive_value * bitor = jive_bitor(2, inputs);
	jive_value * bitxor = jive_bitxor(2, inputs);
	
	for(x_low=-8; x_low<8; x_low++) for(x_high=x_low; x_high<8; x_high++)
	for(y_low=-8; y_low<8; y_low++) for(y_high=y_low; y_high<8; y_high++) {
		jive_bitsymbolicconstant_set_value_range_numeric(node_x, x_low, x_high);
		jive_bitsymbolicconstant_set_value_range_numeric(node_y, y_low, y_high);
		
		const jive_bitstring_value_range * range;
		
		range = jive_value_bits_get_value_range(upperbit_x);
		assert(range->high == (x_high>>3));
		
		range = jive_value_bits_get_value_range(ext_x);
		fflush(stdout);
		assert(range->low == x_low);
		assert(range->high == x_high);
		
		range = jive_value_bits_get_value_range(slice);
		assert(range->low == (x_low>>1));
		assert(range->high == (x_high>>1));
		
		range = jive_value_bits_get_value_range(neg);
		assert(range->low == -x_high);
		assert(range->high == -x_low);
		
		range = jive_value_bits_get_value_range(sum);
		assert(range->low == x_low+y_low);
		assert(range->high == x_high+y_high);
		
		range = jive_value_bits_get_value_range(product);
		
		int p_low = min(min(x_low*y_low, x_low*y_high), min(x_high*y_low, x_high*y_high));
		int p_high = max(max(x_low*y_low, x_low*y_high), max(x_high*y_low, x_high*y_high));
		
		assert(range->low == p_low);
		assert(range->high == p_high);
		
		if (x_low != x_high) continue;
		if (y_low != y_high) continue;
		
		/* only a single value for both x and y, must therefore match
		exactly */
		
		range = jive_value_bits_get_value_range(bitand);
		assert(range->low == (x_low&y_low) && range->high == (x_low&y_low));
		range = jive_value_bits_get_value_range(bitor);
		assert(range->low == (x_low|y_low) && range->high == (x_low|y_low));
		range = jive_value_bits_get_value_range(bitxor);
		assert(range->low == (x_low^y_low) && range->high == (x_low^y_low));
	}
	
	return 0;
}


