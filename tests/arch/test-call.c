/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/arch/address-transform.h>
#include <jive/arch/addresstype.h>
#include <jive/arch/call.h>
#include <jive/arch/memlayout-simple.h>
#include <jive/types/bitstring.h>
#include <jive/view.h>
#include <jive/vsdg/node-private.h>

#include "testnodes.h"

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::addr::type addr;
	jive::bits::type bits16(16);
	jive_node * top = jive_test_node_create(graph->root_region, {}, {}, {&addr, &bits16, &addr});

	const jive::base::type * tmparray1[] = {&bits16, &addr, &addr};

	jive_node * call = jive_call_by_address_node_create(graph->root_region,
		top->outputs[0], NULL,
		2, &top->outputs[1],
		3, tmparray1);
	JIVE_DEBUG_ASSERT(call->noutputs == 3);

	jive_node * bottom = jive_test_node_create(graph->root_region,
		{&bits16, &addr, &addr}, {call->outputs[0], call->outputs[1], call->outputs[2]}, {&addr});
	jive_graph_export(graph, bottom->outputs[0]);

	jive_view(graph, stdout);

	jive_memlayout_mapper_simple mapper;
	jive_memlayout_mapper_simple_init(&mapper, 32);

	jive_node_address_transform(call, &mapper.base);

	jive_memlayout_mapper_simple_fini(&mapper);

	jive_graph_prune(graph);
	jive_view(graph, stdout);
	
	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-call", test_main);
