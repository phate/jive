/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testtypes.h"

#include <assert.h>
#include <locale.h>

#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/node-private.h>

#include "testnodes.h"

static int test_main(void)
{
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_region * region = graph->root_region;
	jive_test_state_type type;
	jive_test_value_type value_type;
	const jive::base::type * tmparray0[] = {&type};
	
	jive_node * n1 = jive_test_node_create(region,
		0, NULL, NULL,
		1, tmparray0);
	
	bool error_handler_called = false;
	try {
		const jive::base::type * tmparray1[] = {&value_type};
		jive::output * tmparray2[] = {n1->outputs[0]};
		jive_test_node_create(region,
			1, tmparray1, tmparray2,
			0, 0);
	} catch (jive::type_error e) {
		error_handler_called = true;
	}
	
	assert(error_handler_called);

	jive_graph_destroy(graph);
	jive_context_destroy(ctx);
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/test-typemismatch", test_main);
