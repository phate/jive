#include <assert.h>
#include <jive/graph.h>

void
jive_graph_check_depth_from_root(jive_graph * graph)
{
	jive_graph_traverser * trav = jive_graph_traverse_topdown(graph);
	jive_node * node;
	
	while( (node = jive_graph_traverse_next(trav)) != 0) {
		jive_input_edge_iterator i;
		size_t tmp = 0;
		JIVE_ITERATE_INPUTS(i, node) {
			if (i->origin.node->depth_from_root+1 > tmp)
				tmp = i->origin.node->depth_from_root+1;
		}
		assert(node->depth_from_root == tmp);
	}
	jive_graph_traverse_finish(trav);
}
