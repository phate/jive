#include <assert.h>
#include <stdio.h>
#include <locale.h>
#include <jive/vsdg.h>
#include <jive/view.h>

#include <jive/vsdg/node-private.h>

static bool
graph_contains_node(jive_graph * graph, jive_node * node)
{
	bool found = false;
	jive_traverser * trav = jive_topdown_traverser_create(graph);
	
	jive_node * tmp;
	
	while( (tmp = jive_traverser_next(trav)) != 0) {
		found = found || (tmp == node);
	}
	
	jive_traverser_destroy(trav);
	
	return found;
}

int main()
{
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_region * region = graph->root_region;
	JIVE_DECLARE_TYPE(type);
	jive_node * n1 = jive_node_create(region,
		0, NULL, NULL,
		1, (const jive_type *[]){type});
	
	jive_node * n2 = jive_node_create(region,
		1, (const jive_type *[]){type}, &n1->outputs[0],
		1, (const jive_type *[]){type});
	
	jive_node * n3 = jive_node_create(region,
		1, (const jive_type *[]){type}, &n2->outputs[0],
		1, (const jive_type *[]){type});
	
	jive_node * bottom = jive_node_create(region,
		2, (const jive_type *[]){type, type}, (jive_output *[]){n2->outputs[0], n3->outputs[0]},
		0, NULL);
	
	jive_node_reserve(bottom);
	
	jive_node * n4 = jive_node_create(region,
		1, (const jive_type *[]){type}, &n1->outputs[0],
		1, (const jive_type *[]){type});
	
	jive_output_replace(n2->outputs[0], n4->outputs[0]);
	assert(n2->outputs[0]->users.first == 0);
	
	jive_graph_prune(graph);
	
	assert(! graph_contains_node(graph, n2) );
	
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	
	return 0;
}
