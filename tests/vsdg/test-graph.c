/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
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

static int test_main(void)
{
	setlocale(LC_ALL, "");
	jive_graph * graph = jive_graph_create();
	
	jive_region * region = graph->root_region;
	
	jive_test_value_type type;
	jive_node * n1 = jive_test_node_create(region, {}, {}, {&type});
	assert(n1);
	assert(n1->depth() == 0);

	jive_node * n2 = jive_test_node_create(region, {&type}, {n1->output(0)}, {});
	assert(n2);
	assert(n2->depth() == 1);
	
	jive_graph_destroy(graph);
	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/test-graph", test_main);
