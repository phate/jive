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
	jive::graph graph;

	static const jive::bits::type bits8(8);
	static const jive::bits::type bits16(16);
	static const jive::bits::type bits32(32);

	static const jive::valuetype * decl_elems[] = {&bits8, &bits16, &bits32};
	std::shared_ptr<const jive::rcd::declaration> rcddecl(
		new jive::rcd::declaration({&bits8, &bits16, &bits32}));
	static jive::rcd::type rcdtype(rcddecl);
	
	static const jive::unn::declaration unndecl = {3, decl_elems};
	static jive::unn::type unntype(&unndecl);

	static const jive::unn::declaration empty_unndecl = {0, NULL};
	static jive::unn::type empty_unntype(&empty_unndecl);

	jive::memtype memtype;
	jive::addrtype addrtype;
	auto top = jive::test::simple_node_create(graph.root(), {}, {},
		{addrtype, memtype, bits8, bits16, bits32, memtype, addrtype});

	auto state = top->output(1);
	auto states0 = jive_store_by_address_create(top->output(0), &bits32, top->output(4), 1, &state);

	jive::output * tmparray1[] = {top->output(2), top->output(3), top->output(4)};
	auto group = jive_group_create(rcddecl, 3, tmparray1);
	jive::output * tmparray2[] = {top->output(1), top->output(5)};
	auto states1 = jive_store_by_address_create(top->output(0), &rcdtype, group, 2, tmparray2);

	auto unify = jive_unify_create(&unndecl, 2, top->output(4));
	auto states2 = jive_store_by_address_create(top->output(0), &unntype, unify, 1, &state);

	auto states3 = jive_store_by_address_create(top->output(6), &bits32, top->output(4), 1, &state);
	auto states4 = jive_store_by_address_create(top->output(0), &bits32, top->output(4), 1,
		&states3[0]);

	unify = jive_empty_unify_create(graph.root(), &empty_unndecl);
	auto states5 = jive_store_by_address_create(top->output(0), &empty_unntype, unify, 1, &state);

	auto bottom = jive::test::simple_node_create(graph.root(),
		std::vector<jive::port>(6, memtype),
		{states0[0], states1[0], states1[0], states2[0], states4[0], states5[0]},
		{memtype});
	graph.export_port(bottom->output(0), "dummy");

	graph.normalize();
	graph.prune();

	jive::view(graph.root(), stderr);

	assert(states3[0]->node()->input(2)->origin()->node() == top);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-store", test_main);
