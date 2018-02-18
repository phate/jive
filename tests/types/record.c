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
#include <jive/types/record.h>
#include <jive/view.h>


static int _test_rcdgroup(void)
{
	using namespace jive;

	jive::graph graph;
	
	auto dcl = rcddeclaration::create(&graph, {&bit8, &bit16, &bit32});
	jive::rcdtype rcdtype(dcl);

	auto edcl = rcddeclaration::create(&graph);
	jive::rcdtype rcdtype_empty(edcl);

	auto i0 = graph.add_import(bit8, "");
	auto i1 = graph.add_import(bit16, "");
	auto i2 = graph.add_import(bit32, "");

	jive::output * tmparray1[] = {i0, i1, i2};

	auto g0 = jive_group_create(dcl, 3, tmparray1);
	auto g1 = jive_empty_group_create(&graph, edcl);

	graph.add_export(g0, "");
	graph.add_export(g1, "");

	graph.normalize();
	graph.prune();

	jive::view(graph.root(), stderr);

	assert(g0->node()->operation() != g1->node()->operation());

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/record/test-rcdgroup", _test_rcdgroup)

static int _test_rcdselect()
{
	using namespace jive;

	jive::graph graph;
	auto dcl = rcddeclaration::create(&graph, {&bit8, &bit16, &bit32});
	jive::rcdtype rcdtype(dcl);

	auto a1 = graph.add_import(bit8, "");
	auto a2 = graph.add_import(bit16, "");
	auto a3 = graph.add_import(bit32, "");
	auto a4 = graph.add_import(rcdtype, "");
	auto a5 = graph.add_import(addrtype(rcdtype), "");

	std::vector<jive::output*> args({a1, a2, a3});
	auto g0 = jive_group_create(dcl, 3, &args[0]);
	auto load = addrload_op::create(a5, rcdtype, {});

	auto s0 = select_op::create(a4, 1);
	auto s1 = select_op::create(g0, 1);
	auto s2 = select_op::create(a4, 2);
	auto s3 = select_op::create(load, 0);

	graph.add_export(s0, "");
	graph.add_export(s1, "");
	graph.add_export(s2, "");
	graph.add_export(s3, "");

	graph.normalize();
	graph.prune();

	jive::view(graph.root(), stderr);

	assert(is_select_node(s0->node()));
	assert(s1 == a2);
	assert(is_select_node(s2->node()));
	assert(!dynamic_cast<const load_op*>(&s3->node()->input(0)->origin()->node()->operation()));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/record/test-rcdselect", _test_rcdselect)
