/*
 * Copyright 2010 2011 2012 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testtypes.h"

#include <assert.h>

#include <jive/view.h>
#include <jive/vsdg.h>

#include "testnodes.h"

void test_basic_traversal(jive_graph * graph, jive::node * n1, jive::node * n2)
{
	jive::node * tmp;

	{
		jive::bottomup_traverser trav(graph->root());
		tmp = trav.next();
		assert(tmp == graph->root()->bottom());
		tmp = trav.next();
		assert(tmp==n2);
		tmp = trav.next();
		assert(tmp==n1);
		tmp = trav.next();
		assert(tmp==0);
	}
}

void test_order_enforcement_traversal()
{
	jive_graph graph;
	
	jive_test_value_type type;
	jive::node * n1 = jive_test_node_create(graph.root(), {}, {}, {&type, &type});
	jive::node * n2 = jive_test_node_create(graph.root(), {&type}, {n1->output(0)}, {&type});
	jive::node * n3 = jive_test_node_create(graph.root(),
		{&type, &type}, {n2->output(0), n1->output(1)}, {&type});

	jive::node * tmp;

	{
		jive::bottomup_traverser trav(graph.root());

		tmp = trav.next();
		assert(tmp==n3);
		tmp = trav.next();
		assert(tmp==n2);
		tmp = trav.next();
		assert(tmp == graph.root()->bottom());
		tmp = trav.next();
		assert(tmp==n1);
		tmp = trav.next();
		assert(tmp==0);
	}
}

static int test_main(void)
{
	jive_graph graph;
	jive_test_value_type type;
	jive::node * n1 = jive_test_node_create(graph.root(), {}, {}, {&type, &type});
	jive::node * n2 = jive_test_node_create(graph.root(),
		{&type, &type}, {n1->output(0), n1->output(1)}, {&type});

	graph.root()->bottom()->add_input(&type, n2->output(0));
	(void)n1;
	(void)n2;
	
	/*
		The &graph now looks like the following:
		
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
	
	test_basic_traversal(&graph, n1, n2);
	test_basic_traversal(&graph, n1, n2);
	test_order_enforcement_traversal();

	test_basic_traversal(&graph, n1, n2);
	
	assert(!graph.has_active_traversers());

	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/traverser/test-graphtraverser", test_main);
