/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>

#include <jive/arch/address-transform.h>
#include <jive/arch/addresstype.h>
#include <jive/types/bitstring.h>
#include <jive/view.h>
#include <jive/vsdg.h>

#include "testnodes.h"

static int test_main(void)
{
	jive_graph graph;

	jive::addr::type addrtype;
	jive::bits::type bits32(32);
	jive::bits::type bits64(64);
	jive::node * top = jive_test_node_create(graph.root(),
		{}, {}, {&addrtype, &bits32, &bits64});

	auto b0 = jive_address_to_bitstring_create(top->output(0), 32, &top->output(0)->type());
	auto a0 = jive_bitstring_to_address_create(b0, 32, &addrtype);

	auto a1 = jive_bitstring_to_address_create(top->output(1), 32, &addrtype);
	auto b1 = jive_address_to_bitstring_create(a1, 32, &addrtype);

	jive::node * bottom = jive_test_node_create(graph.root(),
		{&addrtype, &bits32}, {a0, b1}, {});

	assert(bottom->input(0)->origin() == top->output(0));
	assert(bottom->input(1)->origin() == top->output(1));

	auto b2 = dynamic_cast<jive::output*>(
		jive_bitstring_to_address_create(top->output(1), 32, &addrtype));
	auto b3 = dynamic_cast<jive::output*>(
		jive_bitstring_to_address_create(top->output(1), 32, &addrtype));
	auto a2 = dynamic_cast<jive::output*>(
		jive_address_to_bitstring_create(top->output(0), 32, &top->output(0)->type()));
	auto a3 = dynamic_cast<jive::output*>(
		jive_address_to_bitstring_create(top->output(0), 32, &top->output(0)->type()));
	
	assert(a2->node()->operation() == a3->node()->operation());
	assert(b2->node()->operation() == b3->node()->operation());

	auto b4 = dynamic_cast<jive::output*>(
		jive_bitstring_to_address_create(top->output(2), 64, &addrtype));
	auto a4 = dynamic_cast<jive::output*>(
		jive_address_to_bitstring_create(top->output(0), 64, &addrtype));

	assert(a2->node()->operation() != a4->node()->operation());
	assert(b2->node()->operation() != b4->node()->operation());
	
	jive::view(graph.root(), stderr);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/address-transform/test-address-transform-nodes", test_main);
