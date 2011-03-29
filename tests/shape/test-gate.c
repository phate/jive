#include <assert.h>
#include <locale.h>
#include <jive/context.h>
#include <jive/arch/registers.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/vsdg/resource-private.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg.h>

int main()
{
	setlocale(LC_ALL, "");
	jive_context * ctx = jive_context_create();
	
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_region * region = graph->root_region;
	
	JIVE_DECLARE_TYPE(type);
	
	jive_node * node = jive_node_create(region,
		0, NULL, NULL,
		0, NULL);
	
	jive_gate * g1 = jive_type_create_gate(type, graph, "g1");
	jive_gate * g2 = jive_type_create_gate(type, graph, "g2");
	jive_gate * g3 = jive_type_create_gate(type, graph, "g3");
	
	jive_node_gate_output(node, g1);
	jive_node_gate_output(node, g2);
	jive_output * o = jive_node_gate_output(node, g3);
	
	jive_variable * v1 = jive_variable_create(graph);
	jive_variable * v2 = jive_variable_create(graph);
	jive_variable * v3 = jive_variable_create(graph);
	
	jive_variable_assign_gate(v1, g1);
	jive_variable_assign_gate(v2, g2);
	
	jive_shaped_graph * shaped_graph = jive_shaped_graph_create(graph);
	
	assert(jive_shaped_variable_interferes_with(jive_shaped_graph_map_variable(shaped_graph, v1), jive_shaped_graph_map_variable(shaped_graph, v2)));
	
	jive_variable_assign_gate(v3, g3);
	
	assert(jive_shaped_variable_interferes_with(jive_shaped_graph_map_variable(shaped_graph, v1), jive_shaped_graph_map_variable(shaped_graph, v3)));
	assert(jive_shaped_variable_interferes_with(jive_shaped_graph_map_variable(shaped_graph, v2), jive_shaped_graph_map_variable(shaped_graph, v3)));
	
	jive_variable_unassign_gate(v1, g1);
	
	assert(!jive_shaped_graph_map_variable(shaped_graph, v1));
	
	jive_output_destroy(o);
	
	assert(!jive_shaped_variable_interferes_with(jive_shaped_graph_map_variable(shaped_graph, v2), jive_shaped_graph_map_variable(shaped_graph, v3)));
	
	jive_node_gate_output(node, g3);
	
	assert(jive_shaped_variable_interferes_with(jive_shaped_graph_map_variable(shaped_graph, v2), jive_shaped_graph_map_variable(shaped_graph, v3)));
	
	jive_shaped_graph_destroy(shaped_graph);
	
	jive_graph_destroy(graph);
	
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	return 0;
}
