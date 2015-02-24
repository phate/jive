/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
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
	const jive::base::type * tmparray0[] = {&type};
	jive_node * n1 = jive_test_node_create(region,
		0, NULL, NULL,
		1, tmparray0);
	const jive::base::type * tmparray1[] = {&type};
	const jive::base::type * tmparray2[] = {&type};
	
	jive_node * n2 = jive_test_node_create(region,
		1, tmparray1, &n1->outputs[0],
		1, tmparray2);
	const jive::base::type * tmparray3[] = {&type};
	const jive::base::type * tmparray4[] = {&type};
	
	jive_node * n3 = jive_test_node_create(region,
		1, tmparray3, &n2->outputs[0],
		1, tmparray4);
	const jive::base::type * tmparray5[] = {&type, &type};
	jive::output * tmparray6[] = {n2->outputs[0], n3->outputs[0]};
	
	jive_node * bottom = jive_test_node_create(region,
		2, tmparray5, tmparray6,
		1, tmparray5);
	
	jive_graph_export(graph, bottom->outputs[0]);
	const jive::base::type * tmparray7[] = {&type};
	const jive::base::type * tmparray8[] = {&type};
	
	jive_node * n4 = jive_test_node_create(region,
		1, tmparray7, &n1->outputs[0],
		1, tmparray8);
	
	jive_output_replace(n2->outputs[0], n4->outputs[0]);
	assert(n2->outputs[0]->users.first == 0);
	
	jive_graph_prune(graph);
	
	assert(! graph_contains_node(graph, n2) );
	
	jive_graph_destroy(graph);
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/test-prune-replace", test_main);
