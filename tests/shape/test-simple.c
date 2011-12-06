#include <assert.h>
#include <stdio.h>
#include <locale.h>
#include <jive/vsdg.h>
#include <jive/view.h>

#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/shaped-region.h>
#include <jive/types/bitstring.h>

int main()
{
	setlocale(LC_ALL, "");
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_output * y = jive_bitsymbolicconstant(graph, 8, "y");
	jive_output_auto_assign_variable(y);
	
	jive_shaped_graph * shaped_graph = jive_shaped_graph_create(graph);
	
	assert(jive_shaped_graph_map_region(shaped_graph, graph->root_region) != NULL);
	assert(jive_shaped_graph_map_ssavar(shaped_graph, y->ssavar) != NULL);
	
	jive_region * subregion = jive_region_create_subregion(graph->root_region);
	assert(jive_shaped_graph_map_region(shaped_graph, subregion) != NULL);
	
	jive_output * o = jive_bitsymbolicconstant(graph, 8, "x");
	jive_bitslice(o, 0, 4);
	jive_variable * var = jive_variable_create(graph);
	jive_ssavar * ssavar = jive_ssavar_create(o, var);
	
	assert(jive_shaped_graph_map_variable(shaped_graph, var) == NULL);
	assert(jive_shaped_graph_map_ssavar(shaped_graph, ssavar) == NULL);
	
	jive_ssavar_assign_output(ssavar, o);
	assert(jive_shaped_graph_map_variable(shaped_graph, var) != NULL);
	assert(jive_shaped_graph_map_ssavar(shaped_graph, ssavar) != NULL);
	
	jive_shaped_region * shaped_root_region = jive_shaped_graph_map_region(shaped_graph, graph->root_region);
	jive_cut * c1 = jive_shaped_region_create_cut(shaped_root_region);
	jive_cut * c2 = jive_shaped_region_create_cut(shaped_root_region);
	
	assert(c2->region_cut_list.next == c1);
	
	jive_cut_append(c1, y->node);
	jive_cut_append(c2, o->node);
	
	jive_shaped_graph_destroy(shaped_graph);
	
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	return 0;
}
