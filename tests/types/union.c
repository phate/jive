/*
 * Copyright 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * Copyright 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testnodes.h"

#include <assert.h>

#include <jive/arch/addresstype.h>
#include <jive/arch/load.h>
#include <jive/rvsdg.h>
#include <jive/types/bitstring.h>
#include <jive/types/union.h>
#include <jive/view.h>


static int test_unnchoose(void)
{
	using namespace jive;

	jive::graph graph;

	addrtype at;
	bittype bt8(8);
	bittype bt16(16);
	bittype bt32(32);

	auto dcl = unndeclaration::create(&graph, {&bt8, &bt16, &bt32});
	jive::unntype unntype(dcl);

	auto i0 = graph.add_import(bt8, "");
	auto i1 = graph.add_import(unntype, "");
	auto i2 = graph.add_import(unntype, "");
	auto i3 = graph.add_import(at, "");

	auto u0 = jive_unify_create(dcl, 0, i0);
	auto load = addrload_op::create(i3, unntype, {});

	auto c0 = choose_op::create(i1, 1);
	auto c1 = choose_op::create(u0, 0);
	auto c2 = choose_op::create(i2, 1);
	auto c3 = choose_op::create(load, 0);

	graph.add_export(c0, "");
	auto x1 = graph.add_export(c1, "");
	graph.add_export(c2, "");
	auto x3 = graph.add_export(c3, "");

	graph.normalize();
	graph.prune();

	jive::view(graph.root(), stderr);

	assert(x1->origin() == i0);
	assert(c0->node()->operation() == c2->node()->operation());
	assert(dynamic_cast<const jive::load_op *>(&x3->origin()->node()->operation()));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/union/test-unnchoose", test_unnchoose)

static int test_unnunify(void)
{
	using namespace jive;

	jive::graph graph;
	
	bittype bits8(8);
	bittype bits16(16);
	bittype bits32(32);

	auto dcl = unndeclaration::create(&graph, {&bits8, &bits16, &bits32});
	jive::unntype unntype(dcl);

	auto edcl = unndeclaration::create(&graph);
	jive::unntype unntype_empty(edcl);
	
	auto top = jive::test::simple_node_create(graph.root(), {}, {}, {bits8});

	auto u0 = jive_unify_create(dcl, 0, top->output(0));
	auto u1 = jive_empty_unify_create(graph.root(), edcl);

	auto bottom = jive::test::simple_node_create(graph.root(), {unntype, unntype_empty}, {u0, u1},
		{bits8});
	graph.add_export(bottom->output(0), "dummy");

	graph.normalize();
	graph.prune();

	jive::view(graph.root(), stderr);

	assert(u0->node()->operation() != u1->node()->operation());

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/union/test-unnunify", test_unnunify)
