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
	
	JIVE_DECLARE_TYPE(type);
	jive_node * n1 = jive_node_create(graph->root_region,
		0, NULL, NULL,
		2, (const jive_type *[]){type, type});
	
	jive_node * n2 = jive_node_create(graph->root_region,
		2, (const jive_type *[]){type, type}, n1->outputs,
		0, NULL);
	
	/*
		The graph now looks like the following:
		
		╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴
		╷┏━━━━━━━━━━━━━━┓╵
		╷┃              ┃╵
		╷┠──────────────┨╵
		╷┃      n1      ┃╵
		╷┠──────┬───────┨╵
		╷┃ #0:X │ #1:X  ┃╵
		╷┗━━━┯━━┷━━━┯━━━┛╵
		╷    │      │    ╵
		╷    │      │    ╵
		╷┏━━━┷━━┯━━━┷━━━┓╵
		╷┃ #0:X │ #1:X  ┃╵
		╷┠──────┴───────┨╵
		╷┃      n2      ┃╵
		╷┠──────────────┨╵
		╷┃ #0:X         ┃╵
		╷┗━━━━━━━━━━━━━━┛╵
		╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╵
	*/
	
	jive_resource * r0 = jive_type_create_resource(type, graph);
	jive_resource * r1 = jive_type_create_resource(type, graph);
	
	jive_resource_assign_output(r0, n1->outputs[0]);
	jive_resource_assign_output(r1, n1->outputs[1]);
	assert(jive_resource_is_active_after(r0, n1) == 1);
	assert(jive_resource_is_active_after(r1, n1) == 1);
	/* only one interference, as outputs for n1 */
	assert(jive_resource_interferes_with(r0, r1) == 1);
	
	jive_resource_assign_input(r0, n2->inputs[0]);
	jive_resource_assign_input(r1, n2->inputs[1]);
	assert(jive_resource_is_active_before(r0, n2) == 1);
	assert(jive_resource_is_active_before(r1, n2) == 1);
	/* they interfere twice, as output to n1 and as input to n2 */
	assert(jive_resource_interferes_with(r0, r1) == 2);
	
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	return 0;
}
