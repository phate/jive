/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/arch/address-transform.h>
#include <jive/arch/addresstype.h>
#include <jive/types/bitstring.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/node-private.h>

#include "testnodes.h"

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::addr::type addrtype;
	jive::bits::type bits32(32);
	jive::bits::type bits64(64);
	jive_node * top = jive_test_node_create(graph->root_region,
		{}, {}, {&addrtype, &bits32, &bits64});

	jive::output * b0 = jive_address_to_bitstring_create(top->outputs[0], 32,
		&top->outputs[0]->type());
	jive::output * a0 = jive_bitstring_to_address_create(b0, 32, &addrtype);

	jive::output * a1 = jive_bitstring_to_address_create(top->outputs[1], 32, &addrtype);
	jive::output * b1 = jive_address_to_bitstring_create(a1, 32, &addrtype);

	jive_node * bottom = jive_test_node_create(graph->root_region,
		{&addrtype, &bits32}, {a0, b1}, {});

	assert(bottom->input(0)->origin() == top->outputs[0]);
	assert(bottom->input(1)->origin() == top->outputs[1]);

	jive::output * b2 = jive_bitstring_to_address_create(top->outputs[1], 32, &addrtype);
	jive::output * b3 = jive_bitstring_to_address_create(top->outputs[1], 32, &addrtype);
	jive::output * a2 = jive_address_to_bitstring_create(top->outputs[0], 32,
		&top->outputs[0]->type());
	jive::output * a3 = jive_address_to_bitstring_create(top->outputs[0], 32,
		&top->outputs[0]->type());
	
	assert(a2->node()->operation() == a3->node()->operation());
	assert(b2->node()->operation() == b3->node()->operation());

	jive::output * b4 = jive_bitstring_to_address_create(top->outputs[2], 64, &addrtype);
	jive::output * a4 = jive_address_to_bitstring_create(top->outputs[0], 64, &addrtype);

	assert(a2->node()->operation() != a4->node()->operation());
	assert(b2->node()->operation() != b4->node()->operation());
	
	jive_view(graph, stderr);

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/address-transform/test-address-transform-nodes", test_main);
