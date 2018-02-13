/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>

#include <jive/arch/address-transform.h>
#include <jive/arch/address.h>
#include <jive/arch/memlayout-simple.h>
#include <jive/rvsdg.h>
#include <jive/types/bitstring.h>
#include <jive/view.h>

#include "testnodes.h"

static int test_main(void)
{
	using namespace jive;

	jive::graph graph;

	addrtype at;
	auto dcl = rcddeclaration::create(&graph, {&bit32, &bit32});

	auto i0 = graph.add_import(at, "");
	auto i1 = graph.add_import(at, "");

	auto memb1 = memberof_op::create(i0, dcl, 0);
	auto memb2 = memberof_op::create(i0, dcl, 1);
	
	auto cont1 = containerof_op::create(memb1, dcl, 0);
	auto cont2 = containerof_op::create(memb2, dcl, 0);
	
	auto cont3 = containerof_op::create(i1, dcl, 0);
	
	auto memb3 = memberof_op::create(cont3, dcl, 0);
	auto memb4 = memberof_op::create(cont3, dcl, 1);
	
	jive::view(graph.root(), stdout);
	
	assert(cont1 == i0);
	assert(cont2 != i0);
	
	assert(memb4 != i1);
	assert(memb3 == i1);
	
	auto zero = create_bitconstant(graph.root(), "00000000000000000000000000000000");
	auto one = create_bitconstant(graph.root(), "10000000000000000000000000000000");
	auto minus_one = create_bitconstant(graph.root(), "11111111111111111111111111111111");
	
	auto a0 = jive_arraysubscript(i0, &bit32, zero);
	//assert(a0 == top->outputs[0]);
	auto a1 = jive_arraysubscript(i0, &bit32, one);
	assert(a1 != i0);
	jive_arraysubscript(a1, &bit32, minus_one);
	jive::view(graph.root(), stdout);
	//assert(tmp == a0);
	
	jive_arrayindex(a1, a0, &bit32, &bit32);
	//assert(diff == one);
	
	auto diff2 = jive_arrayindex(i0, i1, &bit32, &bit32);

	auto memberof = memberof_op::create(cont3, dcl, 1);
	auto arraysub = jive_arraysubscript(i0, &bit32, one);

	graph.add_export(memberof, "");
	graph.add_export(arraysub, "");
	graph.add_export(diff2, "");

	memlayout_mapper_simple mapper(4);
	transform_address(cont3->node(), mapper);
	transform_address(memberof->node(), mapper);
	transform_address(diff2->node(), mapper);
	transform_address(arraysub->node(), mapper);

	graph.prune();
	jive::view(graph.root(), stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-address", test_main)
