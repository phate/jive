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

static int
test_memberof()
{
	using namespace jive;

	jive::graph graph;
	auto dcl = rcddeclaration::create(&graph, {&bit32, &bit32});

	auto i0 = graph.add_import(addrtype::instance(), "");

	auto m1 = memberof_op::create(i0, dcl, 0);
	auto m2 = memberof_op::create(i0, dcl, 0);
	auto m3 = memberof_op::create(i0, dcl, 1);

	auto c1 = containerof_op::create(m1, dcl, 0);

	auto ex0 = graph.add_export(c1, "");
	auto ex1 = graph.add_export(m2, "");
	auto ex2 = graph.add_export(m3, "");

	view(graph, stdout);

	assert(ex0->origin() == i0);
	assert(is_memberof_node(ex1->origin()->node()));
	assert(is_memberof_node(ex2->origin()->node()));

	memlayout_mapper_simple mapper(4);
	transform_address(ex1->origin()->node(), mapper);
	transform_address(ex2->origin()->node(), mapper);

	graph.prune();
	view(graph, stdout);

	assert(is_bit2addr_node(ex1->origin()->node()));
	assert(is_bit2addr_node(ex2->origin()->node()));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-memberof", test_memberof)

static int
test_containerof()
{
	using namespace jive;

	jive::graph graph;
	auto dcl = rcddeclaration::create(&graph, {&bit32, &bit32});

	auto i0 = graph.add_import(addrtype::instance(), "");

	auto c1 = containerof_op::create(i0, dcl, 0);
	auto c2 = containerof_op::create(i0, dcl, 0);
	auto c3 = containerof_op::create(i0, dcl, 1);

	auto m1 = memberof_op::create(c1, dcl, 0);

	auto ex0 = graph.add_export(m1, "");
	auto ex1 = graph.add_export(c2, "");
	auto ex2 = graph.add_export(c3, "");

	view(graph, stdout);

	assert(ex0->origin() == i0);
	assert(is_containerof_node(ex1->origin()->node()));
	assert(is_containerof_node(ex2->origin()->node()));

	memlayout_mapper_simple mapper(4);
	transform_address(ex1->origin()->node(), mapper);
	transform_address(ex2->origin()->node(), mapper);

	graph.prune();
	view(graph, stdout);

	assert(is_bit2addr_node(ex1->origin()->node()));
	assert(is_bit2addr_node(ex2->origin()->node()));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-containerof", test_containerof)

static int
test_arraysubscript()
{
	using namespace jive;

	jive::graph graph;

	auto i0 = graph.add_import(addrtype::instance(), "");
	auto i1 = graph.add_import(bit32, "");

	auto as1 = arraysubscript_op::create(i0, bit32, i1);

	auto ex0 = graph.add_export(as1, "");

	view(graph, stdout);

	assert(is_arraysubscript_node(ex0->origin()->node()));

	memlayout_mapper_simple mapper(4);
	transform_address(ex0->origin()->node(), mapper);

	graph.prune();
	view(graph, stdout);

	assert(is_bit2addr_node(ex0->origin()->node()));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-arraysubscript", test_arraysubscript)

static int
test_arrayindex()
{
	using namespace jive;

	jive::graph graph;

	auto i0 = graph.add_import(addrtype::instance(), "");
	auto i1 = graph.add_import(addrtype::instance(), "");

	auto ai1 = arrayindex_op::create(i0, i1, bit32, bit32);

	auto ex0 = graph.add_export(ai1, "");

	view(graph, stdout);

	assert(is_arrayindex_node(ex0->origin()->node()));

	memlayout_mapper_simple mapper(4);
	transform_address(ex0->origin()->node(), mapper);

	graph.prune();
	view(graph, stdout);

	assert(is_bitsdiv_node(ex0->origin()->node()));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-arrayindex", test_arrayindex)
