/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testtypes.h"

#include <assert.h>
#include <locale.h>

#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/statetype.h>

#include "testnodes.h"

static int test_main(void)
{
	setlocale(LC_ALL, "");
	
	jive_graph graph;
	
	jive_test_state_type statetype;
	jive_node * top = jive_test_node_create(graph.root(), {}, {}, {&statetype, &statetype});

	std::vector<jive::oport*> outputs;
	for (size_t n = 0; n < top->noutputs(); n++)
		outputs.push_back(top->output(n));

	auto merged = jive_state_merge(&statetype, 2, &outputs[0]);
	auto split = jive_state_split(&statetype, merged, 2);
	jive_test_node_create(graph.root(), {&statetype, &statetype}, {split[0], split[1]}, {});

	jive_view(&graph, stdout);

	std::unique_ptr<jive_graph> graph2 = graph.copy();
	jive_view(graph2.get(), stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/test-statesplit", test_main);
