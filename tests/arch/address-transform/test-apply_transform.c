/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>

#include <jive/arch/address-transform.h>
#include <jive/arch/addresstype.h>
#include <jive/arch/memlayout-simple.h>
#include <jive/types/function/fctapply.h>
#include <jive/types/function/fcttype.h>
#include <jive/view.h>
#include <jive/vsdg.h>

#include "testnodes.h"

static int
test_main(void)
{
	jive::graph graph;

	jive::addr::type addrtype;
	const jive::base::type * addrptr = &addrtype;
	jive::fct::type fcttype(1, &addrptr, 1, &addrptr);
	jive::node * top = jive_test_node_create(graph.root(), {}, {}, {&fcttype, &addrtype});

	jive::oport * address = top->output(1);
	auto results = jive_apply_create(top->output(0), 1, &address);

	jive::node * bottom = jive_test_node_create(graph.root(), {&addrtype},
		{results.begin(), results.end()}, {});

	jive::view(graph.root(), stdout);

	jive::memlayout_mapper_simple mapper(4);
	jive_node_address_transform(dynamic_cast<jive::output*>(results[0])->node(), &mapper);

	jive::view(graph.root(), stdout);

	assert(dynamic_cast<jive::output*>(bottom->input(0)->origin())->node()->operation()
		== jive::bitstring_to_address_operation(32, addrtype));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/address-transform/test-apply_transform", test_main);
