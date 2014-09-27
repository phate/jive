/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/arch/address-transform.h>
#include <jive/arch/addresstype.h>
#include <jive/arch/call.h>
#include <jive/types/bitstring.h>
#include <jive/view.h>
#include <jive/vsdg/node-private.h>

#include "testnodes.h"

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	jive::addr::type addr;
	jive::bits::type bits16(16);
	const jive::base::type * tmparray0[] = {&addr, &bits16, &addr};
	jive_node * top = jive_test_node_create(graph->root_region,
		0, NULL, NULL,
		3, tmparray0);
	const jive::base::type * tmparray1[] = {&bits16, &addr, &addr};

	jive_node * call = jive_call_by_address_node_create(graph->root_region,
		top->outputs[0], NULL,
		2, &top->outputs[1],
		3, tmparray1);
	JIVE_DEBUG_ASSERT(call->noutputs == 3);
	const jive::base::type * tmparray2[] = {&bits16, &addr, &addr};

	jive_node * bottom = jive_test_node_create(graph->root_region,
		3, tmparray2, &call->outputs[0],
		1, tmparray0);
	jive_graph_export(graph, bottom->outputs[0]);

	jive_view(graph, stdout);

	jive_call_node_address_transform(jive_call_node_cast(call), 32);

	jive_graph_prune(graph);
	jive_view(graph, stdout);
	
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-call", test_main);
