/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testtypes.h"

#include <assert.h>
#include <locale.h>

#include <jive/context.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/statetype.h>

#include "testnodes.h"

static int test_main(void)
{
	setlocale(LC_ALL, "");
	
	jive_context * context = jive_context_create();
	
	jive_graph * graph = jive_graph_create();
	
	jive_test_state_type statetype;
	const jive::base::type * tmparray0[] = {&statetype, &statetype};
	
	jive_node * top = jive_test_node_create(graph->root_region,
		0, NULL, NULL,
		2, tmparray0);
	
	jive::output * merged = jive_state_merge(&statetype, 2, &top->outputs[0]);
	
	std::vector<jive::output *> split = jive_state_split(&statetype, merged, 2);
	const jive::base::type * tmparray1[] = {&statetype, &statetype};

	jive::output * split_states[2] = {split[0], split[1]};
	jive_node * bottom = jive_test_node_create(graph->root_region,
		2, tmparray1, split_states,
		0, NULL);
	(void) bottom;
	
	jive_view(graph, stdout);
	
	jive_context * context2 = jive_context_create();
	jive_graph * graph2 = jive_graph_copy(graph);
	
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
