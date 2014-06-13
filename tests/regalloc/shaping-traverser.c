/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <stdio.h>
#include <locale.h>
#include <jive/vsdg.h>
#include <jive/view.h>
#include <jive/regalloc/shaping-traverser.h>

#include <jive/vsdg/node-private.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_region * region = graph->root_region;
	
	JIVE_DECLARE_TYPE(type);
	JIVE_DECLARE_CONTROL_TYPE(ctrl);
	const jive::base::type * tmparray0[] = {type};
	
	jive_node * n1 = jive_test_node_create(region,
		0, NULL, NULL,
		1, tmparray0);
		
	jive_region * sub = jive_region_create_subregion(region);
	const jive::base::type * tmparray1[] = {ctrl};
	
	jive_node * n2 = jive_test_node_create(sub,
		0, NULL, NULL,
		1, tmparray1);
	const jive::base::type * tmparray2[] = {type, ctrl};
	jive::output * tmparray3[] = {n1->outputs[0], n2->outputs[0]};
	
	jive_node * n3 = jive_test_node_create(region,
		2, tmparray2,
		tmparray3,
		0, NULL);
	
	jive_graph_export(graph, n3->outputs[0]);
	
	jive_shaping_region_traverser * regtrav = jive_shaping_region_traverser_create(graph);
	
	jive_traverser * root_trav = jive_shaping_region_traverser_enter_region(regtrav, region);
	assert(root_trav->frontier.first->node == n3);
	
	jive_traverser * sub_trav = jive_shaping_region_traverser_enter_region(regtrav, sub);
	assert(sub_trav->frontier.first == 0);
	
	jive_cut_append(jive_region_create_cut(region), n3);
	assert(root_trav->frontier.first->node == n1);
	assert(sub_trav->frontier.first->node == n2);
	
	jive_shaping_region_traverser_destroy(regtrav);
	
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	return 0;
}

JIVE_UNIT_TEST_REGISTER("regalloc/shaping-traverser", test_main);
