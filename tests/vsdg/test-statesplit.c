/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testtypes.h"

#include <assert.h>

#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/statetype.h>

#include "testnodes.h"

static int test_main(void)
{
	jive::graph graph;
	
	jive::test::statetype statetype;
	auto top = jive::test::simple_node_create(graph.root(), {}, {}, {&statetype, &statetype});

	std::vector<jive::output*> outputs;
	for (size_t n = 0; n < top->noutputs(); n++)
		outputs.push_back(top->output(n));

	auto merged = jive_state_merge(&statetype, 2, &outputs[0]);
	auto split = jive_state_split(&statetype, merged, 2);
	jive::test::simple_node_create(graph.root(), {&statetype, &statetype}, {split[0], split[1]}, {});

	jive::view(graph.root(), stdout);

	std::unique_ptr<jive::graph> graph2 = graph.copy();
	jive::view(graph2->root(), stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/test-statesplit", test_main);
