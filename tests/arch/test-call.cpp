/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.hpp"

#include <assert.h>

#include <jive/arch/address-transform.hpp>
#include <jive/arch/addresstype.hpp>
#include <jive/arch/call.hpp>
#include <jive/arch/memlayout-simple.hpp>
#include <jive/types/bitstring.hpp>
#include <jive/types/function.hpp>
#include <jive/view.hpp>

#include "testnodes.hpp"

static int test_main(void)
{
	using namespace jive;

	jive::addrtype at(bit16);

	jive::graph graph;
	auto i0 = graph.add_import({addrtype(fcttype({&bit16, &at}, {&bit16, &at, &at})), ""});
	auto i1 = graph.add_import({bit16, ""});
	auto i2 = graph.add_import({at, ""});

	auto call = addrcall_op::create(i0, {i1, i2}, {&bit16, &at, &at}, nullptr);
	JIVE_DEBUG_ASSERT(call.size() == 3);

	graph.add_export(call[0], {call[0]->type(), ""});
	graph.add_export(call[1], {call[1]->type(), ""});
	graph.add_export(call[2], {call[2]->type(), ""});

	jive::view(graph.root(), stdout);

	memlayout_mapper_simple mapper(4);
	transform_address(node_output::node(call[0]), mapper);
	graph.prune();

	jive::view(graph.root(), stdout);
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-call", test_main)
