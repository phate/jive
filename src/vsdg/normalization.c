#include <jive/vsdg/normalization.h>
#include <jive/vsdg/normalization-helpers.h>
#include <jive/vsdg/traverser.h>

void
jive_graph_normalize(jive_graph * graph)
{
	jive_traverser * trav = jive_topdown_traverser_create(graph);
	jive_node * node;
	while( (node = jive_traverser_next(trav)) != 0) {
		jive_node * repl = jive_node_normalize(node);
		if (repl != node) jive_node_destroy(node);
	}
	jive_traverser_destroy(trav);
}
