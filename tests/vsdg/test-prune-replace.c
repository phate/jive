/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testtypes.h"

#include <assert.h>
#include <locale.h>
#include <stdio.h>

#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/node-private.h>

#include "testnodes.h"

static bool
graph_contains_node(jive_graph * graph, jive_node * node)
{
	bool found = false;
	
	for (jive_node * tmp : jive::topdown_traverser(graph)) {
		found = found || (tmp == node);
	}
	
	return found;
}

static int test_main(void)
{
	jive_graph * graph = jive_graph_create();
	
	jive_region * region = graph->root_region;
	jive_test_value_type type;
	jive_node * n1 = jive_test_node_create(region, {}, {}, {&type});
	jive_node * n2 = jive_test_node_create(region, {&type}, {n1->outputs[0]}, {&type});
	jive_node * n3 = jive_test_node_create(region, {&type}, {n2->outputs[0]}, {&type});
	jive_node * bottom = jive_test_node_create(region,
		{&type, &type}, {n2->outputs[0], n3->outputs[0]}, {&type});
	
	jive_graph_export(graph, bottom->outputs[0]);
	
	jive_node * n4 = jive_test_node_create(region, {&type}, {n1->outputs[0]}, {&type});

	n2->outputs[0]->replace(n4->outputs[0]);
	assert(n2->outputs[0]->users.first == 0);
	
	jive_graph_prune(graph);
	
	assert(! graph_contains_node(graph, n2) );
	
	jive_graph_destroy(graph);
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/test-prune-replace", test_main);
