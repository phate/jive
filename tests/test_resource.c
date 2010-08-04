#include <assert.h>
#include <stdio.h>
#include <locale.h>
#include <jive/vsdg.h>

#include <jive/vsdg/node-private.h>

int main()
{
	setlocale(LC_ALL, "");
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_region * region = graph->root_region;
	
	JIVE_DECLARE_TYPE(type);
	
	jive_node * n1 = jive_node_create(region,
		0, NULL, NULL,
		1, (const jive_type *[]){type});
	
	jive_node * n2 = jive_node_create(region,
		1, (const jive_type *[]){type}, &n1->outputs[0],
		0, NULL);
	
	jive_resource * resource = jive_type_create_resource(type, graph);
	jive_resource_assign_output(resource, n1->outputs[0]);
	jive_resource_assign_input(resource, n2->inputs[0]);
	
	assert(jive_resource_is_active_before(resource, n2) == 1);
	assert(jive_resource_is_active_after(resource, n1) == 1);
	
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	return 0;
}
