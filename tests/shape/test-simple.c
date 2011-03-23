#include <assert.h>
#include <stdio.h>
#include <locale.h>
#include <jive/vsdg.h>
#include <jive/view.h>

#include <jive/regalloc/shaped-graph.h>

int main()
{
	setlocale(LC_ALL, "");
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_shaped_graph * shaped_graph = jive_shaped_graph_create(graph);
	
	assert(jive_shaped_graph_map_region(shaped_graph, graph->root_region) != NULL);
	
	jive_region * subregion = jive_region_create_subregion(graph->root_region);
	
	assert(jive_shaped_graph_map_region(shaped_graph, subregion) != NULL);
	
	jive_shaped_graph_destroy(shaped_graph);
	
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	return 0;
}
