/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/vsdg.h>
#include <jive/vsdg/node-private.h>
#include <jive/arch/memorytype.h>

#include "testnodes.h"

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	jive_node * node0 = jive_test_node_create(graph->root_region, 0, NULL, NULL, 0, NULL);
	jive_node * node1 = jive_test_node_create(graph->root_region, 0, NULL, NULL, 0, NULL);

	jive::mem::type memtype;
	jive_gate * arg_gate = memtype.create_gate(graph, "arg");
	jive_gate * ret_gate = memtype.create_gate(graph, "ret");

	jive_output * arg = jive_node_gate_output(node0, arg_gate);
	jive_input * ret = jive_node_gate_input(node1, ret_gate, arg);

	assert(dynamic_cast<jive::state::output*>(arg));
	assert(dynamic_cast<jive::state::input*>(ret));
	assert(dynamic_cast<jive::state::gate*>(arg_gate));
	assert(dynamic_cast<jive::state::type*>(&memtype) != nullptr);

	jive_graph_destroy(graph);
	jive_context_assert_clean(context);
	jive_context_destroy(context);		

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-memory-type", test_main);
