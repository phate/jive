/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/arch/address-transform.h>
#include <jive/arch/addresstype.h>
#include <jive/arch/call.h>
#include <jive/arch/memlayout-simple.h>
#include <jive/types/bitstring.h>
#include <jive/view.h>
#include <jive/vsdg/node-private.h>

#include "testnodes.h"

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_graph graph;

	jive::addr::type addr;
	jive::bits::type bits16(16);
	jive_node * top = jive_test_node_create(graph.root(), {}, {}, {&addr, &bits16, &addr});

	const jive::base::type * tmparray1[] = {&bits16, &addr, &addr};

	std::vector<jive::output*> tmp = {top->output(1), top->output(2)};
	jive_node * call = jive_call_by_address_node_create(graph.root(),
		top->output(0), NULL, 2, &tmp[0], 3, tmparray1);
	JIVE_DEBUG_ASSERT(call->noutputs() == 3);

	jive_node * bottom = jive_test_node_create(graph.root(),
		{&bits16, &addr, &addr}, {call->output(0), call->output(1), call->output(2)}, {&addr});
	graph.export_port(bottom->output(0), "dummy");

	jive_view(&graph, stdout);

	jive::memlayout_mapper_simple mapper(4);

	jive_node_address_transform(call, &mapper);

	graph.prune();
	jive_view(&graph, stdout);
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-call", test_main);
