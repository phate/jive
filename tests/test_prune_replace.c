#include <assert.h>
#include <jive/graph.h>
#include "testnodes.h"
#include <jive/graphdebug.h>

static bool
graph_contains_node(jive_graph * graph, jive_node * node)
{
	bool found = false;
	jive_graph_traverser * trav = jive_graph_traverse_topdown(graph);
	
	jive_node * tmp;
	
	while( (tmp = jive_graph_traverse_next(trav)) != 0) {
		found = found || (tmp == node);
	}
	
	jive_graph_traverse_finish(trav);
	
	return found;
}

int main()
{
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_node * n1 = test_node_create(graph, 0, 0);
	test_value * value1 = test_node_value(n1);
	
	jive_node * n2 = test_node_create(graph, 1, &value1);
	test_value * value2 = test_node_value(n2);
	jive_node * n3 = test_node_create(graph, 1, &value1);
	test_value * value3 = test_node_value(n3);
	
	jive_node * bottom = test_node_create(graph, 2, (test_value *[]){value2, value3});
	
	jive_node_reserve(bottom);
	
	jive_node * n4 = test_node_create(graph, 1, &value1);
	test_value * value4 = test_node_value(n4);
	
	jive_value_replace((jive_value *)value2, (jive_value *)value4);
	jive_output_edge_iterator i = jive_node_iterate_outputs(n2);
	assert(i == 0);
	
	jive_graph_prune(graph);
	
	assert(! graph_contains_node(graph, n2) );
	
	jive_context_destroy(ctx);
	
	return 0;
}
