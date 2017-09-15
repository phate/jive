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
#include <jive/vsdg/statemux.h>

#include "testnodes.h"

static int
test_main(void)
{
	jive::test::statetype st;

	jive::graph graph;
	auto x = graph.import(st, "x");
	auto y = graph.import(st, "y");

	auto merged = jive::create_state_merge(st, {x, y});
	auto split = jive::create_state_split(st, merged, 2);

	graph.export_port(split[0], "x");
	graph.export_port(split[1], "y");

	jive::view(graph.root(), stdout);
	std::unique_ptr<jive::graph> graph2 = graph.copy();
	jive::view(graph2->root(), stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/test-statemux", test_main);
