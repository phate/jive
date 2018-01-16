/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <string.h>

#include <jive/arch/address-transform.h>
#include <jive/arch/address.h>
#include <jive/arch/linker-symbol.h>
#include <jive/arch/memlayout-simple.h>
#include <jive/rvsdg/graph.h>
#include <jive/rvsdg/label.h>
#include <jive/view.h>

#include "testnodes.h"

static int test_main(void)
{
	using namespace jive;

	jive::graph graph;

	jive_linker_symbol bla_symbol;
	jive_linker_symbol foobar_symbol;
	jive::external_label bla("bla", &bla_symbol);
	jive::external_label foobar("foobar", &foobar_symbol);

	auto o0 = jive_label_to_address_create(graph.root(), &foobar);
	auto o1 = jive_label_to_address_create(graph.root(), &bla);

	const jive::address::label_to_address_op * attrs0;
	const jive::address::label_to_address_op * attrs1;
	attrs0 = dynamic_cast<const jive::address::label_to_address_op*>(&o0->node()->operation());
	attrs1 = dynamic_cast<const jive::address::label_to_address_op*>(&o1->node()->operation());

	assert(attrs0->label() == &foobar);
	assert(attrs1->label() == &bla);
	
	assert(o0->node()->operation() != *attrs1);
	
	auto o2 = jive_label_to_bitstring_create(graph.root(), &foobar, 32);
	auto o3 = jive_label_to_bitstring_create(graph.root(), &bla, 32);
	auto o4 = jive_label_to_bitstring_create(graph.root(), &foobar, 16);

	const jive::address::label_to_bitstring_op * attrs2;
	const jive::address::label_to_bitstring_op * attrs3;
	const jive::address::label_to_bitstring_op * attrs4;
	attrs2 = dynamic_cast<const jive::address::label_to_bitstring_op*>(&o2->node()->operation());
	attrs3 = dynamic_cast<const jive::address::label_to_bitstring_op*>(&o3->node()->operation());
	attrs4 = dynamic_cast<const jive::address::label_to_bitstring_op*>(&o4->node()->operation());

	assert(attrs2->label() == &foobar);
	assert(attrs3->label() == &bla);
	assert(attrs4->label() == &foobar);
	
	assert(o2->node()->operation() != *attrs4);
	assert(o2->node()->operation() != *attrs3);
	
	jive::addrtype addr;
	bittype bits32(32);
	bittype bits16(16);
	auto bottom = jive::test::simple_node_create(graph.root(),
		{addr, addr, bits32, bits32, bits16}, {o0, o1, o2, o3, o4}, {addr});
	graph.add_export(bottom->output(0), "dummy");

	jive::view(graph.root(), stderr);

	memlayout_mapper_simple mapper(4);
	transform_address(o0->node(), mapper);
	transform_address(o1->node(), mapper);

	graph.prune();
	jive::view(graph.root(), stderr);

	return 0;
}


JIVE_UNIT_TEST_REGISTER("arch/test-label-nodes", test_main)
