#include <assert.h>
#include <jive/graph.h>
#include <jive/nodeclass.h>
#include "testnodes.h"
#include "graph-sanity-checks.h"

void test_basic_traversal(jive_graph * graph, jive_node * n1, jive_node * n2)
{
	jive_graph_traverser * trav;
	jive_node *tmp;
	
	trav = jive_graph_traverse_topdown(graph);
	
	tmp = jive_graph_traverse_next(trav);
	assert(tmp==n1);
	tmp = jive_graph_traverse_next(trav);
	assert(tmp==n2);
	
	jive_graph_traverse_finish(trav);
	
	trav = jive_graph_traverse_bottomup(graph);
	
	tmp = jive_graph_traverse_next(trav);
	assert(tmp==n2);
	tmp = jive_graph_traverse_next(trav);
	assert(tmp==n1);
	
	jive_graph_traverse_finish(trav);
	
	jive_graph_check_depth_from_root(graph);
}

void test_traversal_insertion(jive_graph * graph, jive_node * n1, jive_node * n2)
{
	jive_graph_traverser * trav;
	jive_node * node;
	
	trav = jive_graph_traverse_topdown(graph);
	
	node = jive_graph_traverse_next(trav);
	assert(node==n1);
	
	jive_node * n3 = test_node_create(graph, 0, 0);
	
	test_value * out = test_node_value(n3);
	jive_node * n4 = test_node_create(graph, 1, &out);
	
	jive_node * n5 = test_node_create(graph, 0, 0);
	jive_state_edge_create(n2, n5);
	
	bool visited_n3 = false, visited_n4 = false, visited_n5 = false;
	for(;;) {
		node = jive_graph_traverse_next(trav);
		if (!node) break;
		if (node==n3) visited_n3 = true;
		if (node==n4) visited_n4 = true;
		if (node==n5) visited_n5 = true;
	}
	
	jive_graph_traverse_finish(trav);
	
	assert(!visited_n3);
	assert(!visited_n4);
	assert(visited_n5);
	
	jive_graph_prune(graph);
	jive_graph_check_depth_from_root(graph);
}

int main()
{
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_node * n1 = test_node_create(graph, 0, 0);
	test_value * out = test_node_value(n1);
	
	jive_node * n2 = test_node_create(graph, 1, &out);
	
	jive_state_edge_create(n1, n2);
	
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
	
	jive_node_reserve(n2);
	
	test_basic_traversal(graph, n1, n2);
	test_basic_traversal(graph, n1, n2);
	
	test_traversal_insertion(graph, n1, n2);
	
	test_basic_traversal(graph, n1, n2);
	
	jive_graph_check_depth_from_root(graph);
	jive_context_destroy(ctx);
	
	return 0;
}
