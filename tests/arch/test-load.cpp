/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.hpp"

#include <assert.h>

#include <jive/arch/addresstype.hpp>
#include <jive/arch/load.hpp>
#include <jive/arch/store.hpp>
#include <jive/rvsdg.hpp>
#include <jive/types/bitstring/type.hpp>
#include <jive/view.hpp>

#include "testnodes.hpp"

static int test_main()
{
	using namespace jive;

	jive::graph graph;
	auto i0 = graph.add_import({addrtype(bit32), ""});
	auto i1 = graph.add_import({addrtype(bit32), ""});
	auto i2 = graph.add_import({memtype(), ""});
	auto i3 = graph.add_import({bit32, ""});

	auto load0 = addrload_op::create(i0, {i2});

	auto states = addrstore_op::create(i1, i3, {i2});
	auto load1 = addrload_op::create(i1, {states[0]});
	assert(load1 == i3);

	graph.add_export(load0, {load0->type(), ""});
	auto ex1 = graph.add_export(load1, {load1->type(), ""});

	graph.normalize();
	graph.prune();

	jive::view(graph.root(), stderr);

	assert(ex1->origin() == i3);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-load", test_main)
