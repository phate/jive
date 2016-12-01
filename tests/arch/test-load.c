/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/arch/addresstype.h>
#include <jive/arch/load.h>
#include <jive/arch/memorytype.h>
#include <jive/arch/store.h>
#include <jive/types/bitstring/type.h>
#include <jive/view.h>
#include <jive/vsdg.h>

#include "testnodes.h"

static int test_main()
{
	setlocale(LC_ALL, "");

	jive_graph graph;

	jive::mem::type memtype;
	jive::addr::type addrtype;
	jive::bits::type bits32(32);
	jive::node * top = jive_test_node_create(graph.root(), {}, {},
		{&addrtype, &addrtype, &memtype, &bits32});

	jive::oport * state = top->output(2);
	auto load0 = jive_load_by_address_create(top->output(0), &bits32, 1, &state);

	state = top->output(2);
	auto states = jive_store_by_address_create(top->output(1), &bits32, top->output(3), 1, &state);
	auto load1 = jive_load_by_address_create(top->output(1), &bits32, 1, &states[0]);
	assert(load1 == top->output(3));

	jive::node * bottom = jive_test_node_create(graph.root(),
		{&bits32, &bits32}, {load0, load1}, {&addrtype});
	graph.export_port(bottom->output(0), "dummy");

	graph.normalize();
	graph.prune();

	jive_view(&graph, stderr);

	assert(dynamic_cast<jive::output*>(bottom->input(1)->origin())->node() == top);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-load", test_main);
