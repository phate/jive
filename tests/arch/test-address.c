/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>

#include <jive/arch/address-transform.h>
#include <jive/arch/address.h>
#include <jive/arch/memlayout-simple.h>
#include <jive/types/bitstring.h>
#include <jive/view.h>
#include <jive/vsdg.h>

#include "testnodes.h"

static int test_main(void)
{
	jive::graph graph;

	jive::bits::type bits32(32);
	std::shared_ptr<const jive::rcd::declaration> rec(new jive::rcd::declaration({&bits32, &bits32}));

	jive::addr::type addrtype;
	auto top = jive::test::simple_node_create(graph.root(), {}, {}, {addrtype, addrtype});
	
	auto memb1 = jive_memberof(top->output(0), rec, 0);
	auto memb2 = jive_memberof(top->output(0), rec, 1);
	
	auto cont1 = jive_containerof(memb1, rec, 0);
	auto cont2 = jive_containerof(memb2, rec, 0);
	
	auto cont3 = jive_containerof(top->output(1), rec, 0);
	
	auto memb3 = jive_memberof(cont3, rec, 0);
	auto memb4 = jive_memberof(cont3, rec, 1);
	
	jive::view(graph.root(), stdout);
	
	assert(cont1 == top->output(0));
	assert(cont2 != top->output(0));
	
	assert(memb4 != top->output(1));
	assert(memb3 == top->output(1));
	
	auto zero = jive_bitconstant(graph.root(), 32, "00000000000000000000000000000000");
	auto one = jive_bitconstant(graph.root(), 32, "10000000000000000000000000000000");
	auto minus_one = jive_bitconstant(graph.root(), 32, "11111111111111111111111111111111");
	
	auto a0 = jive_arraysubscript(top->output(0), &bits32, zero);
	//assert(a0 == top->outputs[0]);
	auto a1 = jive_arraysubscript(top->output(0), &bits32, one);
	assert(a1 != top->output(0));
	jive_arraysubscript(a1, &bits32, minus_one);
	jive::view(graph.root(), stdout);
	//assert(tmp == a0);
	
	jive_arrayindex(a1, a0, &bits32, &bits32);
	//assert(diff == one);
	
	auto diff2 = jive_arrayindex(top->output(0), top->output(1), &bits32, &bits32);

	jive::memlayout_mapper_simple mapper(4);

	auto memberof = jive_memberof(cont3, rec, 1);
	auto arraysub = jive_arraysubscript(top->output(0), &bits32, one);

	auto bottom = jive::test::simple_node_create(graph.root(),
		{addrtype, addrtype, bits32}, {memberof, arraysub, diff2}, {addrtype});
	graph.export_port(bottom->output(0), "dummy");

	jive_node_address_transform(cont3->node(), &mapper);
	jive_node_address_transform(memberof->node(), &mapper);
	jive_node_address_transform(diff2->node(), &mapper);
	jive_node_address_transform(arraysub->node(), &mapper);

	graph.prune();
	jive::view(graph.root(), stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-address", test_main);
