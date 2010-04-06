#include <assert.h>
#include <jive/graph.h>
#include <jive/nodeclass.h>

#include "testnodes.h"

int main()
{
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_node * n1 = test_node_create(graph, 0, 0);
	test_value * out = test_node_value(n1);
	
	jive_node * n2 = test_node_create(graph, 1, &out);
	
	jive_edge * state_edge = jive_state_edge_create(n1, n2);
	
	/*
		The graph now looks like the following:
		
		[n1]
		:  |
		:  |
		v  v
		[n2]
		
		where the left one is a state edge and the right one is
		a value edge.
	*/
	
	
	jive_input_edge_iterator i;
	jive_output_edge_iterator o;
	
	i = jive_node_iterate_inputs(n1);
	o = jive_node_iterate_outputs(n2);
	/* n1 has no inputs, n2 has no outputs */
	assert(!i && !o);
	
	/* iterate through output/input edges of n1/n2 */
	o = jive_node_iterate_outputs(n1);
	i = jive_node_iterate_inputs(n2);
	
	/* the state edge must be first */
	assert(o==state_edge && i==state_edge);
	
	o = jive_output_edge_iterator_next(o);
	i = jive_input_edge_iterator_next(i);
	
	/* next must be the value input edge */
	assert(o && i && o==i);
	jive_edge * edge=o;
	assert(edge->origin.node == n1 && edge->target.node==n2);
	assert(edge->origin.port == (jive_value *) test_node_value(n1));
	assert(edge->target.port == (jive_operand *) test_node_operand(n2, 0));
	
	/* no further edges */
	o = jive_output_edge_iterator_next(o);
	i = jive_input_edge_iterator_next(i);
	assert(!o && !i);
	
	jive_context_destroy(ctx);
	
	return 0;
}
