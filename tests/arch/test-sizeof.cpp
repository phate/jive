/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.hpp"

#include <assert.h>

#include <jive/arch/addresstype.hpp>
#include <jive/arch/memlayout-simple.hpp>
#include <jive/arch/sizeof.hpp>
#include <jive/rvsdg/graph.hpp>
#include <jive/rvsdg/traverser.hpp>
#include <jive/types/bitstring.hpp>
#include <jive/types/record.hpp>
#include <jive/types/union.hpp>
#include <jive/view.hpp>

#include "testnodes.hpp"

static int test_main(void)
{
	using namespace jive;

	jive::graph graph;

	bittype bit4(4);
	bittype bit18(18);
	addrtype at(bit32);
	auto rcddcl = rcddeclaration::create({&bit4, &bit8, &bit18});
	auto unndcl = unndeclaration::create(&graph, {&bit4, &bit8, &bit18});

	rcdtype record_t(rcddcl.get());
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

	auto x0 = graph.add_export(s0, {s0->type(), ""});
	auto x1 = graph.add_export(s1, {s1->type(), ""});
	auto x2 = graph.add_export(s2, {s2->type(), ""});
	auto x3 = graph.add_export(s3, {s3->type(), ""});
	auto x4 = graph.add_export(s4, {s4->type(), ""});
	auto x5 = graph.add_export(s5, {s5->type(), ""});
	auto x6 = graph.add_export(s6, {s6->type(), ""});
	auto x7 = graph.add_export(s7, {s7->type(), ""});

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
