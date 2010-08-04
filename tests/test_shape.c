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
	
	JIVE_DECLARE_TYPE(type);
	
	jive_node * n1 = jive_node_create(graph->root_region,
		0, NULL, NULL,
		2, (const jive_type *[]){type, type});
	
	jive_node * n2 = jive_node_create(graph->root_region,
		1, (const jive_type *[]){type}, (jive_output *[]){n1->outputs[0]},
		1, (const jive_type *[]){type});
	
	jive_node * n3 = jive_node_create(graph->root_region,
		2, (const jive_type *[]){type, type}, (jive_output *[]){n2->outputs[0], n1->outputs[1]},
		0, 0);
	
	jive_cut * cut3 = jive_region_create_cut(graph->root_region);
	jive_cut * cut2 = jive_region_create_cut(graph->root_region);
	jive_cut * cut1 = jive_region_create_cut(graph->root_region);
	
	jive_cut_append(cut3, n3);
	jive_cut_append(cut2, n2);
	jive_cut_append(cut1, n1);
	
	jive_resource * r1 = jive_type_create_resource(type, graph);
	jive_resource * r2 = jive_type_create_resource(type, graph);
	jive_resource * r3 = jive_type_create_resource(type, graph);
	jive_resource_assign_output(r1, n1->outputs[0]);
	jive_resource_assign_input(r1, n2->inputs[0]);
	jive_resource_assign_output(r2, n1->outputs[1]);
	jive_resource_assign_input(r2, n3->inputs[1]);
	jive_resource_assign_output(r3, n2->outputs[0]);
	jive_resource_assign_input(r3, n3->inputs[0]);
	
	/*
		The graph with resources assigned to inputs and
		outputs now looks like the folliwng:
		
		   ╷╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴
		   ╷     ┏━━━━━━━━━━━━━━┓╵
		   ╷     ┃              ┃╵
		   ╷     ┠──────────────┨╵
		───┬─────┨      n1      ┠┴─── cut1 ───
		   ╷     ┠──────┬───────┨╵
		   ╷     ┃  r1  │  r2   ┃╵
		   ╷     ┗━━━┯━━┷━━━┯━━━┛╵ after cut1
		   ╷    ┌────┘      │    ╵╶╶╶╶╶╶╶╶╶╶╶╶
		   ╷┏━━━┷━━━━━━━━━━┓│    ╵ before cut2
		   ╷┃  r1          ┃│    ╵
		   ╷┠──────────────┨│    ╵
		───┬┨      n2      ┠┼────┴─── cut2 ───
		   ╷┠──────────────┨│    ╵
		   ╷┃  r3          ┃│    ╵
		   ╷┗━━━┯━━━━━━━━━━┛│    ╵ after cut2
		   ╷    │         ┌─┘    ╵╶╶╶╶╶╶╶╶╶╶╶╶
		   ╷    └──┐      │      ╵ before cut3
		   ╷   ┏━━━┷━━┯━━━┷━━━┓  ╵
		   ╷   ┃  r3  │  r2   ┃  ╵
		   ╷   ┠──────┴───────┨  ╵
		───┬───┨      n3      ┠──┴─── cut3 ───
		   ╷   ┠──────────────┨  ╵
		   ╷   ┃              ┃  ╵
		   ╷   ┗━━━━━━━━━━━━━━┛  ╵
		   ╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╵
	*/
	
	assert(jive_resource_is_active_after(r1, n1) == 1);
	assert(jive_resource_is_active_before(r1, n2) == 1);
	
	assert(jive_resource_is_active_after(r2, n1) == 1);
	assert(jive_resource_is_active_before(r2, n3) == 1);
	/* crosses cut2, therefore node n2 */
	assert(jive_resource_crosses(r2, n2) == 1);
	assert(jive_resource_is_active_before(r2, n2) == 1);
	assert(jive_resource_is_active_after(r2, n2) == 1);
	
	assert(jive_resource_is_active_after(r3, n2) == 1);
	assert(jive_resource_is_active_before(r3, n3) == 1);
	
	assert(jive_resource_interferes_with(r1, r2) == 2); /* after cut1 & before cut2 */
	assert(jive_resource_interferes_with(r2, r3) == 2); /* after cut2 & before cut3 */
	assert(jive_resource_interferes_with(r1, r3) == 0);
	
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	return 0;
}
