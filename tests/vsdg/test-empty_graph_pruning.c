/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>

#include <jive/view.h>
#include <jive/vsdg.h>

static int
test_main(void)
{
	jive::graph graph;

	jive::view(graph.root(), stdout);

	graph.prune();

	assert(graph.root()->nodes.size() == 0);

	jive::view(graph.root(), stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/test-empty_graph_pruning", test_main);
