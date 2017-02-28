/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>

#include <jive/arch/memorytype.h>
#include <jive/vsdg.h>

#include "testnodes.h"

static int test_main(void)
{
	jive::graph graph;

	jive::node * node0 = jive_test_node_create(graph.root(), {}, {}, {});
	jive::node * node1 = jive_test_node_create(graph.root(), {}, {}, {});

	jive::mem::type memtype;
	jive::gate * arg_gate = graph.create_gate(memtype, "arg");
	jive::gate * ret_gate = graph.create_gate(memtype, "ret");

	jive::oport * arg = node0->add_output(arg_gate);
	jive::iport * ret = node1->add_input(ret_gate, arg);

	assert(dynamic_cast<const jive::state::type*>(&arg->type()));
	assert(dynamic_cast<const jive::state::type*>(&ret->type()));
	assert(dynamic_cast<const jive::state::type*>(&arg_gate->type()));
	assert(dynamic_cast<jive::state::type*>(&memtype) != nullptr);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-memory-type", test_main);
