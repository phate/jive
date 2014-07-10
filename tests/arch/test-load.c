/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/arch/addresstype.h>
#include <jive/arch/load.h>
#include <jive/arch/memorytype.h>
#include <jive/arch/store.h>
#include <jive/types/bitstring/type.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/node-private.h>

#include "testnodes.h"

static int test_main()
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	jive::mem::type memtype;
	jive::addr::type addrtype;
	jive::bits::type bits32(32);
	const jive::base::type * tmparray0[] = {&addrtype, &addrtype, &memtype, &bits32};
	jive_node * top = jive_test_node_create(graph->root_region,
		0, NULL, NULL,
		4, tmparray0);

	jive::output * load0 = jive_load_by_address_create(top->outputs[0], &bits32, 1, &top->outputs[2]);

	std::vector<jive::output *> states = jive_store_by_address_create(
		top->outputs[1], &bits32, top->outputs[3], 1, &top->outputs[2]);
	jive::output * load1 = jive_load_by_address_create(top->outputs[1], &bits32, 1, &states[0]);
	assert(load1 == top->outputs[3]);
	const jive::base::type * tmparray1[] = {&bits32, &bits32};
	jive::output * tmparray2[] = {load0, load1};

	jive_node * bottom = jive_test_node_create(graph->root_region,
		2, tmparray1, tmparray2,
		1, tmparray0);
	jive_graph_export(graph, bottom->outputs[0]);

	jive_graph_normalize(graph);
	jive_graph_prune(graph);

	jive_view(graph, stderr);

	assert(bottom->producer(1) == top);

	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-load", test_main);
