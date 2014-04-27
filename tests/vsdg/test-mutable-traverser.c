/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testtypes.h"

#include <assert.h>
#include <stdio.h>
#include <locale.h>
#include <jive/vsdg.h>
#include <jive/view.h>

#include <jive/vsdg/node-private.h>

void test_mutable_traverse_topdown(jive_graph * graph, jive_node * n1, jive_node * n2, jive_node * n3)
{
	jive_traverser * trav = jive_topdown_traverser_create(graph);
	jive_node * tmp;
	bool seen_n1 = false;
	bool seen_n2 = false;
	bool seen_n3 = false;
	
	while( (tmp = jive_traverser_next(trav)) != 0) {
		seen_n1 = seen_n1 || (tmp == n1);
		seen_n2 = seen_n2 || (tmp == n2);
		seen_n3 = seen_n3 || (tmp == n3);
		if (n3->inputs[0]->origin == n1->outputs[0])
			jive_input_divert_origin(n3->inputs[0], n2->outputs[0]);
		else
			jive_input_divert_origin(n3->inputs[0], n1->outputs[0]);
	}
	
	assert(seen_n1);
	assert(seen_n2);
	assert(seen_n3);
	jive_traverser_destroy(trav);
}

static int test_main(void)
{
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_region * region = graph->root_region;
	JIVE_DECLARE_TEST_VALUE_TYPE(type);
	const jive_type * tmparray0[] = {type};
	jive_node * n1 = jive_node_create(region,
		0, NULL, NULL,
		1, tmparray0);
	const jive_type * tmparray1[] = {type};
	
	jive_node * n2 = jive_node_create(region,
		0, NULL, NULL,
		1, tmparray1);
	const jive_type * tmparray2[] = {type};
	const jive_type * tmparray3[] = {type};
	
	jive_node * bottom = jive_node_create(region,
		1, tmparray2, &n1->outputs[0],
		0, tmparray3);
	
	test_mutable_traverse_topdown(graph, n1, n2, bottom);
	
	test_mutable_traverse_topdown(graph, n1, n2, bottom);
	
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	
	/* should also write a test that exercises the bottom-up
	traverser, but since bottom-up traversal is inherently
	more robust I do not have a non-contrived "error scenario"
	yet */
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/test-mutable-traverser", test_main);
