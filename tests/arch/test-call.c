/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>

#include <jive/arch/address-transform.h>
#include <jive/arch/addresstype.h>
#include <jive/arch/call.h>
#include <jive/arch/memlayout-simple.h>
#include <jive/types/bitstring.h>
#include <jive/view.h>

#include "testnodes.h"

static int test_main(void)
{
	jive::graph graph;

	jive::addrtype addr;
	jive::bits::type bits16(16);
	auto top = jive::test::simple_node_create(graph.root(), {}, {}, {addr, bits16, addr});

	const jive::type * tmparray1[] = {&bits16, &addr, &addr};

	std::vector<jive::output*> tmp = {top->output(1), top->output(2)};
	jive::node * call = jive_call_by_address_node_create(graph.root(),
		top->output(0), NULL, 2, &tmp[0], 3, tmparray1);
	JIVE_DEBUG_ASSERT(call->noutputs() == 3);

	auto bottom = jive::test::simple_node_create(graph.root(),
		{bits16, addr, addr}, {call->output(0), call->output(1), call->output(2)}, {addr});
	graph.export_port(bottom->output(0), "dummy");

	jive::view(graph.root(), stdout);

	jive::memlayout_mapper_simple mapper(4);

	jive_node_address_transform(call, &mapper);

	graph.prune();
	jive::view(graph.root(), stdout);
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-call", test_main);
