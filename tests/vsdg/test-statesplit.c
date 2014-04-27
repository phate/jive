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
#include <jive/vsdg/statetype.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");
	
	jive_context * context = jive_context_create();
	
	jive_graph * graph = jive_graph_create(context);
	
	JIVE_DECLARE_TEST_STATE_TYPE(statetype);
	const jive_type * tmparray0[] = {statetype, statetype};
	
	jive_node * top = jive_node_create(graph->root_region,
		0, NULL, NULL,
		2, tmparray0);
	
	jive_output * merged = jive_state_merge(statetype, 2, top->outputs);
	
	jive_node * split = jive_state_split(statetype, merged, 2);
	const jive_type * tmparray1[] = {statetype, statetype};
	
	jive_node * bottom = jive_node_create(graph->root_region,
		2, tmparray1, split->outputs,
		0, NULL);
	(void) bottom;
	
	jive_view(graph, stdout);
	
	jive_context * context2 = jive_context_create();
	jive_graph * graph2 = jive_graph_copy(graph, context2);
	
	jive_graph_destroy(graph);
	
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);
	
	jive_view(graph2, stdout);
	
	jive_graph_destroy(graph2);
	assert(jive_context_is_empty(context2));
	jive_context_destroy(context2);
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/test-statesplit", test_main);
