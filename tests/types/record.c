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
#include <jive/types/bitstring.h>
#include <jive/types/record.h>
#include <jive/view.h>
#include <jive/vsdg.h>


static int _test_rcdgroup(void)
{
	jive::graph graph;
	
	static const jive::bits::type bits8(8);
	static const jive::bits::type bits16(16);
	static const jive::bits::type bits32(32);
	static std::shared_ptr<const jive::rcd::declaration> decl(
		new jive::rcd::declaration({&bits8, &bits16, &bits32}));
	static jive::rcd::type rcdtype(decl);

	auto decl_empty = std::make_shared<const jive::rcd::declaration>();
	jive::rcd::type rcdtype_empty(decl_empty);

	auto top = jive::test::simple_node_create(graph.root(), {}, {}, {bits8, bits16, bits32});
	jive::output * tmparray1[] = {top->output(0), top->output(1), top->output(2)};

	auto g0 = jive_group_create(decl, 3, tmparray1);
	auto g1 = jive_empty_group_create(&graph, decl_empty);

	auto bottom = jive::test::simple_node_create(graph.root(), {rcdtype, rcdtype_empty}, {g0, g1},
		{bits8});

	graph.export_port(bottom->output(0), "dummy");

	graph.normalize();
	graph.prune();

	jive::view(graph.root(), stderr);

	assert(g0->node()->operation() != g1->node()->operation());

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/record/test-rcdgroup", _test_rcdgroup);

static int _test_rcdselect()
{
	using namespace jive;

	jive::graph graph;

	jive::addr::type addrtype;
	jive::bits::type bits8(8);
	jive::bits::type bits16(16);
	jive::bits::type bits32(32);
	std::shared_ptr<const jive::rcd::declaration> decl(
		new jive::rcd::declaration({&bits8, &bits16, &bits32}));
	jive::rcd::type rcdtype(decl);

	auto a1 = graph.root()->add_argument(nullptr, bits8);
	auto a2 = graph.root()->add_argument(nullptr, bits16);
	auto a3 = graph.root()->add_argument(nullptr, bits32);
	auto a4 = graph.root()->add_argument(nullptr, rcdtype);
	auto a5 = graph.root()->add_argument(nullptr, addrtype);

	std::vector<jive::output*> args({a1, a2, a3});
	auto g0 = jive_group_create(decl, 3, &args[0]);
	auto load = jive_load_by_address_create(a5, &rcdtype, 0, NULL);

	auto s0 = jive_select_create(1, a4);
	auto s1 = jive_select_create(1, g0);
	auto s2 = jive_select_create(2, a4);
	auto s3 = jive_select_create(0, load);

	graph.export_port(s0, "");
	graph.export_port(s1, "");
	graph.export_port(s2, "");
	graph.export_port(s3, "");

	graph.normalize();
	graph.prune();

	jive::view(graph.root(), stderr);

	assert(dynamic_cast<const rcd::select_operation*>(&s0->node()->operation()));
	assert(s1 == a2);
	assert(dynamic_cast<const rcd::select_operation*>(&s2->node()->operation()));
	assert(!dynamic_cast<const load_op*>(&s3->node()->input(0)->origin()->node()->operation()));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/record/test-rcdselect", _test_rcdselect);
