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
	auto dcl = rcddeclaration::create({&bit32, &bit32});

	auto i0 = graph.add_import(addrtype(rcdtype(dcl.get())), "");

	auto m1 = memberof_op::create(i0, dcl.get(), 0);
	auto m2 = memberof_op::create(i0, dcl.get(), 0);
	auto m3 = memberof_op::create(i0, dcl.get(), 1);

	auto c1 = containerof_op::create(m1, dcl.get(), 0);

	auto ex0 = graph.add_export(c1, "");
	auto ex1 = graph.add_export(m2, "");
	auto ex2 = graph.add_export(m3, "");

	view(graph, stdout);

	assert(ex0->origin() == i0);
	assert(is<memberof_op>(ex1->origin()->node()));
	assert(is<memberof_op>(ex2->origin()->node()));

	memlayout_mapper_simple mapper(4);
	transform_address(ex1->origin()->node(), mapper);
	transform_address(ex2->origin()->node(), mapper);

	graph.prune();
	view(graph, stdout);

	assert(is<bit2addr_op>(ex1->origin()->node()));
	assert(is<bit2addr_op>(ex2->origin()->node()));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-memberof", test_memberof)

static int
test_containerof()
{
	using namespace jive;

	jive::graph graph;
	auto dcl = rcddeclaration::create({&bit32, &bit32});

	auto i0 = graph.add_import(addrtype(bit32), "");
	auto i1 = graph.add_import(addrtype(bit32), "");

	auto c1 = containerof_op::create(i0, dcl.get(), 0);
	auto c2 = containerof_op::create(i0, dcl.get(), 0);
	auto c3 = containerof_op::create(i1, dcl.get(), 1);

	auto m1 = memberof_op::create(c1, dcl.get(), 0);

	auto ex0 = graph.add_export(m1, "");
	auto ex1 = graph.add_export(c2, "");
	auto ex2 = graph.add_export(c3, "");

	view(graph, stdout);

	assert(ex0->origin() == i0);
	assert(is<containerof_op>(ex1->origin()->node()));
	assert(is<containerof_op>(ex2->origin()->node()));

	memlayout_mapper_simple mapper(4);
	transform_address(ex1->origin()->node(), mapper);
	transform_address(ex2->origin()->node(), mapper);

	graph.prune();
	view(graph, stdout);

	assert(is<bit2addr_op>(ex1->origin()->node()));
	assert(is<bit2addr_op>(ex2->origin()->node()));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-containerof", test_containerof)

static int
test_arraysubscript()
{
	using namespace jive;

	jive::graph graph;

	auto i0 = graph.add_import(addrtype(bit32), "");
	auto i1 = graph.add_import(bit32, "");

	auto as1 = arraysubscript_op::create(i0, bit32, i1);

	auto ex0 = graph.add_export(as1, "");

	view(graph, stdout);

	assert(is<arraysubscript_op>(ex0->origin()->node()));

	memlayout_mapper_simple mapper(4);
	transform_address(ex0->origin()->node(), mapper);

	graph.prune();
	view(graph, stdout);

	assert(is<bit2addr_op>(ex0->origin()->node()));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-arraysubscript", test_arraysubscript)

static int
test_arrayindex()
{
	using namespace jive;

	jive::graph graph;

	auto i0 = graph.add_import(addrtype(bit32), "");
	auto i1 = graph.add_import(addrtype(bit32), "");

	auto ai1 = arrayindex_op::create(i0, i1, bit32, bit32);

	auto ex0 = graph.add_export(ai1, "");

	view(graph, stdout);

	assert(is<arrayindex_op>(ex0->origin()->node()));

	memlayout_mapper_simple mapper(4);
	transform_address(ex0->origin()->node(), mapper);

	graph.prune();
	view(graph, stdout);

	assert(is<bitsdiv_op>(ex0->origin()->node()));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-arrayindex", test_arrayindex)
