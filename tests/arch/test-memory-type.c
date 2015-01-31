/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/arch/memorytype.h>
#include <jive/vsdg.h>
#include <jive/vsdg/node-private.h>

#include "testnodes.h"

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive_node * node0 = jive_test_node_create(graph->root_region, 0, NULL, NULL, 0, NULL);
	jive_node * node1 = jive_test_node_create(graph->root_region, 0, NULL, NULL, 0, NULL);

	jive::mem::type memtype;
	jive::gate * arg_gate = memtype.create_gate(graph, "arg");
	jive::gate * ret_gate = memtype.create_gate(graph, "ret");

	jive::output * arg = jive_node_gate_output(node0, arg_gate);
	jive::input * ret = jive_node_gate_input(node1, ret_gate, arg);

	assert(dynamic_cast<jive::state::output*>(arg));
	assert(dynamic_cast<const jive::state::type*>(&ret->type()));
	assert(dynamic_cast<jive::state::gate*>(arg_gate));
	assert(dynamic_cast<jive::state::type*>(&memtype) != nullptr);

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-memory-type", test_main);
