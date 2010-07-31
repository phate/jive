#include <assert.h>
#include <stdio.h>
#include <locale.h>
#include <jive/vsdg.h>

#include <jive/vsdg/node-private.h>
#include <jive/view.h>

int main()
{
	setlocale(LC_ALL, "");
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_node * n1 = jive_node_create(graph->root_region,
		0, NULL, NULL,
		2, (const jive_type *[]){&jive_type_singleton, &jive_type_singleton});
	
	jive_node * n2 = jive_node_create(graph->root_region,
		1, (const jive_type *[]){&jive_type_singleton}, (jive_output *[]){n1->outputs[0]},
		1, (const jive_type *[]){&jive_type_singleton});
	
	jive_node * n3 = jive_node_create(graph->root_region,
		2, (const jive_type *[]){&jive_type_singleton, &jive_type_singleton}, (jive_output *[]){n2->outputs[0], n1->outputs[1]},
		0, 0);
	
	jive_resource * r1 = jive_type_create_resource(&jive_type_singleton, graph);
	jive_resource * r2 = jive_type_create_resource(&jive_type_singleton, graph);
	jive_resource * r3 = jive_type_create_resource(&jive_type_singleton, graph);
	jive_resource_assign_output(r1, n1->outputs[0]);
	jive_resource_assign_input(r1, n2->inputs[0]);
	jive_resource_assign_output(r2, n1->outputs[1]);
	jive_resource_assign_input(r2, n3->inputs[1]);
	jive_resource_assign_output(r3, n2->outputs[0]);
	jive_resource_assign_input(r3, n3->inputs[0]);
	
	jive_cut * cut3 = jive_region_create_cut(graph->root_region);
	jive_cut * cut2 = jive_region_create_cut(graph->root_region);
	jive_cut * cut1 = jive_region_create_cut(graph->root_region);
	
	jive_cut_append(cut3, n3);
	assert(jive_resource_is_active_before(r3, n3) == 1);
	assert(jive_resource_is_active_before(r2, n3) == 1);
	
	jive_cut_append(cut1, n1);
	assert(jive_resource_is_active_after(r1, n1) == 1);
	assert(jive_resource_is_active_after(r2, n1) == 1);
	
	jive_cut_append(cut2, n2);
	assert(jive_resource_is_active_before(r1, n2) == 1);
	assert(jive_resource_is_active_after(r3, n2) == 1);
	assert(jive_resource_crosses(r2, n2) == 1);
	assert(jive_resource_is_active_before(r2, n2) == 1);
	assert(jive_resource_is_active_after(r2, n2) == 1);
	
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	return 0;
}
