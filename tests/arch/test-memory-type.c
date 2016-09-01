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

	jive_node * node0 = jive_test_node_create(graph->root_region, {}, {}, {});
	jive_node * node1 = jive_test_node_create(graph->root_region, {}, {}, {});

	jive::mem::type memtype;
	jive::gate * arg_gate = jive_graph_create_gate(graph, "arg", memtype);
	jive::gate * ret_gate = jive_graph_create_gate(graph, "ret", memtype);

	jive::output * arg = jive_node_gate_output(node0, arg_gate);
	jive::input * ret = node1->add_input(ret_gate, arg);

	assert(dynamic_cast<const jive::state::type*>(&arg->type()));
	assert(dynamic_cast<const jive::state::type*>(&ret->type()));
	assert(dynamic_cast<const jive::state::type*>(&arg_gate->type()));
	assert(dynamic_cast<jive::state::type*>(&memtype) != nullptr);

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-memory-type", test_main);
