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

	jive::addrtype addr;
	jive::bits::type bits4(4);
	jive::bits::type bits8(8);
	jive::bits::type bits18(18);
	jive::bits::type bits32(32);
	auto rcddcl = rcddeclaration::create(&graph, {&bits4, &bits8, &bits18});
	unndeclaration unndcl({&bits4, &bits8, &bits18});

	rcdtype record_t(rcddcl);
	unntype union_t(&unndcl);

	auto s0 = jive_sizeof_create(graph.root(), &bits4);
	auto s1 = jive_sizeof_create(graph.root(), &bits8);
	auto s2 = jive_sizeof_create(graph.root(), &bits8);
	auto s3 = jive_sizeof_create(graph.root(), &bits18);
	auto s4 = jive_sizeof_create(graph.root(), &bits32);
	auto s5 = jive_sizeof_create(graph.root(), &addr);
	auto s6 = jive_sizeof_create(graph.root(), &record_t);
	auto s7 = jive_sizeof_create(graph.root(), &union_t);

	assert(s1->node()->operation() == s2->node()->operation());
	assert(s0->node()->operation() != s3->node()->operation());

	auto bottom = jive::test::simple_node_create(graph.root(),
		std::vector<jive::port>(8, bits32), {s0, s1, s2, s3, s4, s5, s6, s7}, {bits32});
	graph.add_export(bottom->output(0), "dummy");

	jive::view(graph.root(), stdout);

	jive::memlayout_mapper_simple layout_mapper(4);
	for (jive::node * node : jive::topdown_traverser(graph.root())) {
		if (dynamic_cast<const jive::sizeof_op *>(&node->operation())) {
			jive_sizeof_node_reduce(node, &layout_mapper);
		}
	}
	graph.prune();

	assert(bottom->input(0)->origin()->node()->operation() == jive::bits::uint_constant_op(32, 1));
	assert(bottom->input(1)->origin()->node()->operation() == jive::bits::uint_constant_op(32, 1));
	assert(bottom->input(2)->origin()->node()->operation() == jive::bits::uint_constant_op(32, 1));
	assert(bottom->input(3)->origin()->node()->operation() == jive::bits::uint_constant_op(32, 4));
	assert(bottom->input(4)->origin()->node()->operation() == jive::bits::uint_constant_op(32, 4));
	assert(bottom->input(5)->origin()->node()->operation() == jive::bits::uint_constant_op(32, 4));
	assert(bottom->input(6)->origin()->node()->operation() == jive::bits::uint_constant_op(32, 8));
	assert(bottom->input(7)->origin()->node()->operation() == jive::bits::uint_constant_op(32, 4));
	
	jive::view(graph.root(), stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-sizeof", test_main)
