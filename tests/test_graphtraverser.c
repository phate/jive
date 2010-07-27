#include <assert.h>
//#include "testnodes.h"
//#include "graph-sanity-checks.h"
#include <locale.h>

#include <jive/vsdg.h>
#include <jive/view.h>

#include <jive/vsdg/node-private.h>
#include <jive/vsdg/traverser-private.h>

void test_basic_traversal(jive_graph * graph, jive_node * n1, jive_node * n2)
{
	jive_traverser * trav;
	jive_node * tmp;
	
	trav = jive_topdown_traverser_create(graph);
	
	tmp = jive_traverser_next(trav);
	assert(tmp==n1);
	tmp = jive_traverser_next(trav);
	assert(tmp==n2);
	tmp = jive_traverser_next(trav);
	assert(tmp==0);
	
	jive_traverser_destroy(trav);
	
	trav = jive_bottomup_traverser_create(graph);
	
	tmp = jive_traverser_next(trav);
	assert(tmp==n2);
	tmp = jive_traverser_next(trav);
	assert(tmp==n1);
	tmp = jive_traverser_next(trav);
	assert(tmp==0);
	
	jive_traverser_destroy(trav);
}

void test_traversal_insertion(jive_graph * graph, jive_node * n1, jive_node * n2)
{
	jive_traverser * trav;
	jive_node * node;
	
	trav = jive_topdown_traverser_create(graph);
	
	node = jive_traverser_next(trav);
	assert(node==n1);
	
	/* At this point, n1 has been visited, now create some nodes */
	
	jive_node * n3 = jive_node_create(graph->root_region,
		0, NULL, NULL,
		1, (const jive_type *[]){&jive_type_singleton});
	
	jive_node * n4 = jive_node_create(graph->root_region,
		1, (const jive_type *[]){&jive_type_singleton}, &n3->outputs[0],
		0, NULL);
	
	jive_node * n5 = jive_node_create(graph->root_region,
		1, (const jive_type *[]){&jive_type_singleton}, &n2->outputs[0],
		0, NULL);
	
	/*
		The graph now looks like the following:
		
		╷╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴
		╷┏━━━━━━━━━━━━━━┓┏━━━━━━━━━━━━━━┓                ╵
		╷┃              ┃┃              ┃                ╵
		╷┠──────────────┨┠──────────────┨                ╵
		╷┃      n1      ┃┃      n3      ┃                ╵
		╷┠──────┬───────┨┠──────────────┨                ╵
		╷┃ #0:X │ #1:X  ┃┃ #0:X         ┃                ╵
		╷┗━━━┯━━┷━━━┯━━━┛┗━━━┯━━━━━━━━━━┛                ╵
		╷    │      │        └───────────────┐           ╵
		╷    │      │                        │           ╵
		╷    │      │                        │           ╵
		╷┏━━━┷━━┯━━━┷━━━┓                ┏━━━┷━━━━━━━━━━┓╵
		╷┃ #0:X │ #1:X  ┃                ┃ #0:X         ┃╵
		╷┠──────┴───────┨                ┠──────────────┨╵
		╷┃      n2 *    ┃                ┃      n4      ┃╵
		╷┠──────────────┨                ┠──────────────┨╵
		╷┃ #0:X         ┃                ┃              ┃╵
		╷┗━━━┯━━━━━━━━━━┛                ┗━━━━━━━━━━━━━━┛╵
		╷    │                                           ╵
		╷┏━━━┷━━━━━━━━━━┓                                ╵
		╷┃ #0:X         ┃                                ╵
		╷┠──────────────┨                                ╵
		╷┃      n5 *    ┃                                ╵
		╷┠──────────────┨                                ╵
		╷┃              ┃                                ╵
		╷┗━━━━━━━━━━━━━━┛                                ╵
		╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╵
		
		The newly created nodes n3 and n4 will not be visited,
		as they were not created as descendants of unvisited
		nodes. n5 must be visited, as n2 has not been visited yet.
	*/
	
	bool visited_n2 = false, visited_n3 = false, visited_n4 = false, visited_n5 = false;
	for(;;) {
		node = jive_traverser_next(trav);
		if (!node) break;
		if (node==n2) visited_n2 = true;
		if (node==n3) visited_n3 = true;
		if (node==n4) visited_n4 = true;
		if (node==n5) visited_n5 = true;
	}
	
	jive_traverser_destroy(trav);
	
	assert(!visited_n3);
	assert(!visited_n4);
	assert(visited_n5);
	
	jive_graph_prune(graph);
}

int main()
{
	setlocale(LC_ALL, "");
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	jive_node * n1 = jive_node_create(graph->root_region,
		0, NULL, NULL,
		2, (const jive_type *[]){&jive_type_singleton, &jive_type_singleton});
	
	jive_node * n2 = jive_node_create(graph->root_region,
		2, (const jive_type *[]){&jive_type_singleton, &jive_type_singleton}, n1->outputs,
		1, (const jive_type *[]){&jive_type_singleton});
	
	(void)n1;
	(void)n2;
	
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
		
		Topdown traversal will encounter n1, n2, in this order.
	*/
	
	jive_node_reserve(n2);
	
	test_basic_traversal(graph, n1, n2);
	test_basic_traversal(graph, n1, n2);
	
	test_traversal_insertion(graph, n1, n2);
	
	test_basic_traversal(graph, n1, n2);
	
	assert(!jive_graph_has_active_traversers(graph));
	
	jive_graph_destroy(graph);
	jive_context_destroy(ctx);
	return 0;
}
