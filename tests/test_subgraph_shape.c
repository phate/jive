#include <assert.h>
#include <stdio.h>
#include <locale.h>
#include <jive/vsdg.h>
#include <jive/view.h>

#include <jive/vsdg/node-private.h>

static void shape(jive_node * node)
{
	jive_cut_append(jive_region_create_cut(node->region), node);
}

int main()
{
	setlocale(LC_ALL, "");
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_region * region = graph->root_region;
	JIVE_DECLARE_TYPE(type);
	JIVE_DECLARE_CONTROL_TYPE(ctrl);
	
	jive_node * top = jive_node_create(region,
		0, NULL, NULL,
		3, (const jive_type *[]){type, type, type});
	
	jive_node * cmp = jive_node_create(region,
		2, (const jive_type *[]){type, type},(jive_output *[]){top->outputs[0], top->outputs[1]},
		1, (const jive_type *[]){type});
	
	jive_region * sub1 = jive_region_create_subregion(region);
	jive_node * alt1 = jive_node_create(sub1,
		1, (const jive_type *[]){type}, (jive_output *[]){top->outputs[0]},
		1, (const jive_type *[]){ctrl});
	
	jive_region * sub2 = jive_region_create_subregion(region);
	jive_node * alt2 = jive_node_create(sub2,
		1, (const jive_type *[]){type}, (jive_output *[]){top->outputs[1]},
		1, (const jive_type *[]){ctrl});
	
	jive_node * gamma = jive_node_create(region,
		3, (const jive_type *[]){type, ctrl, ctrl},(jive_output *[]){cmp->outputs[0], alt1->outputs[0], alt2->outputs[0]},
		1, (const jive_type *[]){type});
	
	jive_node * bottom = jive_node_create(region,
		2, (const jive_type *[]){type, type}, (jive_output *[]){gamma->outputs[0], top->outputs[2]},
		0, NULL);
	
	jive_resource * r1 = jive_type_create_resource(type, graph);
	jive_resource_assign_output(r1, cmp->outputs[0]);
	jive_resource_assign_input(r1, gamma->inputs[0]);
	
	jive_resource * r2 = jive_type_create_resource(type, graph);
	jive_resource_assign_output(r2, top->outputs[0]);
	jive_resource_assign_input(r2, cmp->inputs[0]);
	jive_resource_assign_input(r2, alt1->inputs[0]);
	
	jive_resource * r3 = jive_type_create_resource(type, graph);
	jive_resource_assign_output(r3, top->outputs[1]);
	jive_resource_assign_input(r3, cmp->inputs[1]);
	jive_resource_assign_input(r3, alt2->inputs[0]);
	
	jive_resource * r4 = jive_type_create_resource(type, graph);
	jive_resource_assign_output(r4, top->outputs[2]);
	jive_resource_assign_input(r4, bottom->inputs[1]);
	
	jive_view(graph, stderr);
	
	shape(bottom);
	shape(gamma);
	shape(alt1);
	shape(alt2);
	shape(cmp);
	shape(top);
	assert(jive_resource_crosses(r4, gamma) == 1);
	assert(jive_resource_crosses(r4, alt1) == 1);
	assert(jive_resource_crosses(r4, alt2) == 1);
	assert(jive_resource_crosses(r4, cmp) == 1);
	
	assert(jive_resource_crosses(r1, alt1) == 0);
	assert(jive_resource_crosses(r1, alt2) == 0);
	
	assert(jive_resource_crosses(r2, cmp) == 1);
	assert(jive_resource_crosses(r3, cmp) == 1);
	
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	return 0;
}
