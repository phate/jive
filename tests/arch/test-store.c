/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>

#include <jive/arch/addresstype.h>
#include <jive/arch/store.h>
#include <jive/rvsdg.h>
#include <jive/types/bitstring/type.h>
#include <jive/types/record.h>
#include <jive/types/union.h>
#include <jive/view.h>

#include "testnodes.h"

static int test_main(void)
{
	using namespace jive;

	jive::graph graph;

	static const jive::bits::type bits8(8);
	static const jive::bits::type bits16(16);
	static const jive::bits::type bits32(32);

	auto rcddcl = rcddeclaration::create(&graph, {&bits8, &bits16, &bits32});
	auto unndcl = unndeclaration::create(&graph, {&bits8, &bits16, &bits32});
	auto eunndcl = unndeclaration::create(&graph);

	jive::rcdtype rcdtype(rcddcl);
	jive::unntype unntype(unndcl);
	jive::unntype eunntype(eunndcl);

	jive::memtype memtype;
	jive::addrtype addrtype;
	auto top = jive::test::simple_node_create(graph.root(), {}, {},
		{addrtype, memtype, bits8, bits16, bits32, memtype, addrtype});

	auto state = top->output(1);
	auto states0 = addrstore_op::create(top->output(0), top->output(4), bits32, {state});

	jive::output * tmparray1[] = {top->output(2), top->output(3), top->output(4)};
	auto group = jive_group_create(rcddcl, 3, tmparray1);
	auto states1 = addrstore_op::create(top->output(0), group, rcdtype,
		{top->output(1), top->output(5)});

	auto unify = jive_unify_create(unndcl, 2, top->output(4));
	auto states2 = addrstore_op::create(top->output(0), unify, unntype, {state});

	auto states3 = addrstore_op::create(top->output(6), top->output(4), bits32, {state});
	auto states4 = addrstore_op::create(top->output(0), top->output(4), bits32, {states3});

	unify = jive_empty_unify_create(graph.root(), eunndcl);
	auto states5 = addrstore_op::create(top->output(0), unify, eunntype, {state});

	auto bottom = jive::test::simple_node_create(graph.root(),
		std::vector<jive::port>(6, memtype),
		{states0[0], states1[0], states1[0], states2[0], states4[0], states5[0]},
		{memtype});
	graph.add_export(bottom->output(0), "dummy");

	graph.normalize();
	graph.prune();

	jive::view(graph.root(), stderr);

	assert(states3[0]->node()->input(2)->origin()->node() == top);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-store", test_main)
