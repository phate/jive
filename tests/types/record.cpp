/*
 * Copyright 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * Copyright 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.hpp"
#include "testnodes.hpp"

#include <assert.h>

#include <jive/arch/addresstype.hpp>
#include <jive/arch/load.hpp>
#include <jive/rvsdg.hpp>
#include <jive/types/bitstring.hpp>
#include <jive/types/record.hpp>
#include <jive/view.hpp>


static int _test_rcdgroup(void)
{
	using namespace jive;

	jive::graph graph;
	
	auto dcl = rcddeclaration::create({&bit8, &bit16, &bit32});
	jive::rcdtype rcdtype(dcl.get());

	auto edcl = rcddeclaration::create();
	jive::rcdtype rcdtype_empty(edcl.get());

	auto i0 = graph.add_import({bit8, ""});
	auto i1 = graph.add_import({bit16, ""});
	auto i2 = graph.add_import({bit32, ""});

	auto g0 = group_op::create(dcl.get(), {i0, i1, i2});
	auto g1 = group_op::create(&graph, edcl.get());

	graph.add_export(g0, {g0->type(), ""});
	graph.add_export(g1, {g1->type(), ""});

	graph.normalize();
	graph.prune();

	jive::view(graph.root(), stderr);

	assert(node_output::node(g0)->operation() != node_output::node(g1)->operation());

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/record/test-rcdgroup", _test_rcdgroup)

static int _test_rcdselect()
{
	using namespace jive;

	jive::graph graph;
	auto dcl = rcddeclaration::create({&bit8, &bit16, &bit32});
	jive::rcdtype rcdtype(dcl.get());

	auto a1 = graph.add_import({bit8, ""});
	auto a2 = graph.add_import({bit16, ""});
	auto a3 = graph.add_import({bit32, ""});
	auto a4 = graph.add_import({rcdtype, ""});
	auto a5 = graph.add_import({addrtype(rcdtype), ""});

	std::vector<jive::output*> args({a1, a2, a3});
	auto g0 = group_op::create(dcl.get(), {a1, a2, a3});
	auto load = addrload_op::create(a5, {});

	auto s0 = select_op::create(a4, 1);
	auto s1 = select_op::create(g0, 1);
	auto s2 = select_op::create(a4, 2);
	auto s3 = select_op::create(load, 0);

	graph.add_export(s0, {s0->type(), ""});
	graph.add_export(s1, {s1->type(), ""});
	graph.add_export(s2, {s2->type(), ""});
	graph.add_export(s3, {s3->type(), ""});

	graph.normalize();
	graph.prune();

	jive::view(graph.root(), stderr);

	assert(is<select_op>(node_output::node(s0)));
	assert(s1 == a2);
	assert(is<select_op>(node_output::node(s2)));
	auto origin = node_output::node(s3)->input(0)->origin();
	assert(!is<load_op>(node_output::node(origin)));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/record/test-rcdselect", _test_rcdselect)
