/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/arch/address-transform.h>
#include <jive/arch/addresstype.h>
#include <jive/arch/memlayout-simple.h>
#include <jive/types/function/fctapply.h>
#include <jive/types/function/fcttype.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/node-private.h>

#include "testnodes.h"

static int
test_main(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::addr::type addrtype;
	const jive::base::type * addrptr = &addrtype;
	jive::fct::type fcttype(1, &addrptr, 1, &addrptr);
	jive_node * top = jive_test_node_create(graph->root_region, {}, {}, {&fcttype, &addrtype});

	std::vector<jive::output *> results = jive_apply_create(top->outputs[0], 1, &top->outputs[1]);

	jive_node * bottom = jive_test_node_create(graph->root_region, {&addrtype}, results, {});

	jive_view(graph, stdout);

	jive_memlayout_mapper_simple mapper;
	jive_memlayout_mapper_simple_init(&mapper, 32);

	jive_node_address_transform(results[0]->node(), &mapper.base.base);

	jive_memlayout_mapper_simple_fini(&mapper);

	jive_view(graph, stdout);

	assert(bottom->producer(0)->operation() == jive::bitstring_to_address_operation(32, addrtype));

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/address-transform/test-apply_transform", test_main);
