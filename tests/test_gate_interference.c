#include <assert.h>
#include <stdio.h>
#include <locale.h>
#include <jive/vsdg.h>
#include <jive/view.h>

#include <jive/vsdg/node-private.h>

int main()
{
	setlocale(LC_ALL, "");
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_region * region = graph->root_region;
	jive_node * top = jive_node_create(region,
		0, NULL, NULL,
		0, NULL);
	jive_node * bottom = jive_node_create(region,
		0, NULL, NULL,
		0, NULL);
	
	JIVE_DECLARE_TYPE(type);
	
	jive_gate * gate1 = jive_type_create_gate(type, graph, "gate1");
	jive_gate * gate2 = jive_type_create_gate(type, graph, "gate2");
	jive_gate * gate3 = jive_type_create_gate(type, graph, "gate3");
	jive_gate * gate4 = jive_type_create_gate(type, graph, "gate4");
	
	jive_resource * r1 = jive_type_create_resource(type, graph);
	jive_resource * r2 = jive_type_create_resource(type, graph);
	jive_resource * r3 = jive_type_create_resource(type, graph);
	jive_resource * r4 = jive_type_create_resource(type, graph);
	
	jive_node_gate_input(bottom, gate1, jive_node_gate_output(top, gate1));
	jive_node_gate_input(bottom, gate2, jive_node_gate_output(top, gate2));
	jive_output * out3 = jive_node_gate_output(top, gate3);
	jive_input * in3 = jive_node_gate_input(bottom, gate3, out3);
	
	jive_resource_assign_gate(r1, gate1);
	jive_resource_assign_gate(r2, gate2);
	jive_resource_assign_gate(r3, gate3);
	jive_resource_assign_gate(r4, gate4);
	
	assert(jive_resource_interferes_with(r1, r3) == 1);
	jive_node_gate_input(bottom, gate4, jive_node_gate_output(top, gate4));
	jive_input_destroy(in3);
	jive_output_destroy(out3);
	
	assert(jive_gate_interferes_with(gate1, gate2) == 2);
	assert(jive_gate_interferes_with(gate1, gate3) == 0);
	assert(jive_gate_interferes_with(gate1, gate4) == 2);
	
	assert(jive_resource_interferes_with(r1, r2) == 1);
	assert(jive_resource_interferes_with(r1, r3) == 0);
	assert(jive_resource_interferes_with(r1, r4) == 1);
	
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	return 0;
}
