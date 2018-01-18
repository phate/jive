/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>

#include <jive/arch/addresstype.h>
#include <jive/arch/memlayout-simple.h>
#include <jive/arch/sizeof.h>
#include <jive/rvsdg/graph.h>
#include <jive/rvsdg/traverser.h>
#include <jive/types/bitstring.h>
#include <jive/types/record.h>
#include <jive/types/union.h>
#include <jive/view.h>

#include "testnodes.h"

static int test_main(void)
{
	using namespace jive;

	jive::graph graph;

	addrtype at;
	bittype bit4(4);
	bittype bit18(18);
	auto rcddcl = rcddeclaration::create(&graph, {&bit4, &bit8, &bit18});
	auto unndcl = unndeclaration::create(&graph, {&bit4, &bit8, &bit18});

	rcdtype record_t(rcddcl);
	unntype union_t(unndcl);

	auto s0 = jive_sizeof_create(graph.root(), &bit4);
	auto s1 = jive_sizeof_create(graph.root(), &bit8);
	auto s2 = jive_sizeof_create(graph.root(), &bit8);
	auto s3 = jive_sizeof_create(graph.root(), &bit18);
	auto s4 = jive_sizeof_create(graph.root(), &bit32);
	auto s5 = jive_sizeof_create(graph.root(), &at);
	auto s6 = jive_sizeof_create(graph.root(), &record_t);
	auto s7 = jive_sizeof_create(graph.root(), &union_t);

	assert(s1->node()->operation() == s2->node()->operation());
	assert(s0->node()->operation() != s3->node()->operation());

	auto x0 = graph.add_export(s0, "");
	auto x1 = graph.add_export(s1, "");
	auto x2 = graph.add_export(s2, "");
	auto x3 = graph.add_export(s3, "");
	auto x4 = graph.add_export(s4, "");
	auto x5 = graph.add_export(s5, "");
	auto x6 = graph.add_export(s6, "");
	auto x7 = graph.add_export(s7, "");

	jive::view(graph.root(), stdout);

	memlayout_mapper_simple layout_mapper(4);
	for (auto node : jive::topdown_traverser(graph.root())) {
		if (dynamic_cast<const jive::sizeof_op*>(&node->operation()))
			jive_sizeof_node_reduce(node, &layout_mapper);
	}
	graph.prune();

	assert(x0->origin()->node()->operation() == uint_constant_op(32, 1));
	assert(x1->origin()->node()->operation() == uint_constant_op(32, 1));
	assert(x2->origin()->node()->operation() == uint_constant_op(32, 1));
	assert(x3->origin()->node()->operation() == uint_constant_op(32, 4));
	assert(x4->origin()->node()->operation() == uint_constant_op(32, 4));
	assert(x5->origin()->node()->operation() == uint_constant_op(32, 4));
	assert(x6->origin()->node()->operation() == uint_constant_op(32, 8));
	assert(x7->origin()->node()->operation() == uint_constant_op(32, 4));
	
	jive::view(graph.root(), stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-sizeof", test_main)
