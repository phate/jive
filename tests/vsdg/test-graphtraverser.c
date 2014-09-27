/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testtypes.h"

#include <assert.h>
#include <locale.h>

#include <jive/vsdg.h>
#include <jive/view.h>

#include <jive/vsdg/node-private.h>
#include <jive/vsdg/traverser-private.h>

#include "testnodes.h"

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
	assert(tmp == graph->root_region->bottom);
	tmp = jive_traverser_next(trav);
	assert(tmp==0);
	
	jive_traverser_destroy(trav);
	
	trav = jive_bottomup_traverser_create(graph);
	
	tmp = jive_traverser_next(trav);
	assert(tmp == graph->root_region->bottom);
	tmp = jive_traverser_next(trav);
	assert(tmp==n2);
	tmp = jive_traverser_next(trav);
	assert(tmp==n1);
	tmp = jive_traverser_next(trav);
	assert(tmp==0);
	
	jive_traverser_destroy(trav);
}

void test_order_enforcement_traversal(jive_context * ctx)
{
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_test_value_type type;
	const jive::base::type * tmparray0[] = {&type, &type};
	
	jive_node * n1 = jive_test_node_create(graph->root_region,
		0, NULL, NULL,
		2, tmparray0);
	const jive::base::type * tmparray1[] = {&type};
	jive::output * tmparray2[] = {n1->outputs[0]};
	const jive::base::type * tmparray3[] = {&type};
	
	jive_node * n2 = jive_test_node_create(graph->root_region,
		1, tmparray1, tmparray2,
		1, tmparray3);
	const jive::base::type * tmparray4[] = {&type, &type};
	jive::output * tmparray5[] = {n2->outputs[0], n1->outputs[1]};
	const jive::base::type * tmparray6[] = {&type};
	
	jive_node * n3 = jive_test_node_create(graph->root_region,
		2, tmparray4, tmparray5,
		1, tmparray6);
	
	jive_traverser * trav;
	jive_node * tmp;
	
	trav = jive_topdown_traverser_create(graph);

	tmp = jive_traverser_next(trav);
	assert(tmp = graph->root_region->bottom);
	tmp = jive_traverser_next(trav);
	assert(tmp==n1);
	tmp = jive_traverser_next(trav);
	assert(tmp==n2);
	tmp = jive_traverser_next(trav);
	assert(tmp==n3);
	tmp = jive_traverser_next(trav);
	assert(tmp==0);
	
	jive_traverser_destroy(trav);
	
	trav = jive_bottomup_traverser_create(graph);
	
	tmp = jive_traverser_next(trav);
	assert(tmp==n3);
	tmp = jive_traverser_next(trav);
	assert(tmp==n2);
	tmp = jive_traverser_next(trav);
	assert(tmp == graph->root_region->bottom);
	tmp = jive_traverser_next(trav);
	assert(tmp==n1);
	tmp = jive_traverser_next(trav);
	assert(tmp==0);
	
	jive_traverser_destroy(trav);
	
	jive_graph_destroy(graph);
}

void test_traversal_insertion(jive_graph * graph, jive_node * n1, jive_node * n2)
{
	jive_traverser * trav;
	jive_node * node;
	
	trav = jive_topdown_traverser_create(graph);
	
	jive_test_value_type type;
	node = jive_traverser_next(trav);
	assert(node==n1);
	const jive::base::type * tmparray7[] = {&type};
	
	/* At this point, n1 has been visited, now create some nodes */
	
	jive_node * n3 = jive_test_node_create(graph->root_region,
		0, NULL, NULL,
		1, tmparray7);
	const jive::base::type * tmparray8[] = {&type};
	
	jive_node * n4 = jive_test_node_create(graph->root_region,
		1, tmparray8, &n3->outputs[0],
		0, NULL);
	const jive::base::type * tmparray9[] = {&type};
	
	jive_node * n5 = jive_test_node_create(graph->root_region,
		1, tmparray9, &n2->outputs[0],
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
	
	assert(visited_n2);
	assert(!visited_n3);
	assert(!visited_n4);
	assert(visited_n5);
	
	jive_graph_prune(graph);
}

static int test_main(void)
{
	setlocale(LC_ALL, "");
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	jive_test_value_type type;
	const jive::base::type * tmparray10[] = {&type, &type};
	jive_node * n1 = jive_test_node_create(graph->root_region,
		0, NULL, NULL,
		2, tmparray10);
	const jive::base::type * tmparray11[] = {&type, &type};
	const jive::base::type * tmparray12[] = {&type};
	
	jive_node * n2 = jive_test_node_create(graph->root_region,
		2, tmparray11, &n1->outputs[0],
		1, tmparray12);

	jive_node_add_input(graph->root_region->bottom, &type, n2->outputs[0]);
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
	
	test_basic_traversal(graph, n1, n2);
	test_basic_traversal(graph, n1, n2);
	test_order_enforcement_traversal(ctx);
	
	test_traversal_insertion(graph, n1, n2);
	
	test_basic_traversal(graph, n1, n2);
	
	assert(!jive_graph_has_active_traversers(graph));
	
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/test-graphtraverser", test_main);
