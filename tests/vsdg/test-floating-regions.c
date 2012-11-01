/*
 * Copyright 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <locale.h>
#include <assert.h>

#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/node-private.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	JIVE_DECLARE_VALUE_TYPE(vtype);
	jive_region * region0 = graph->root_region;
	jive_node * node0 = jive_node_create(region0,
		0, NULL, NULL,
		1, &vtype);

	jive_region * region1 = jive_region_create_subregion(graph->root_region);
	jive_node * node1 = jive_node_create(region1,
		0, NULL, NULL,
		1, &vtype);

	jive_region * region2 = jive_region_create_subregion(region1);
	jive_node * node2 = jive_node_create(region2,
		0, NULL, NULL,
		1, &vtype);


	jive_region * floating = jive_floating_region_create(graph);
	jive_node * fnode0 = jive_node_create(floating,
		1, (const jive_type *[]){vtype},
			(jive_output *[]){node0->outputs[0]},
		1, &vtype);

	jive_view(graph, stdout);
	assert(floating->parent == graph->root_region);

	jive_node * fnode1 = jive_node_create(floating,
		3, (const jive_type *[]){vtype, vtype, vtype},
			(jive_output *[]){node0->outputs[0], node1->outputs[0], fnode0->outputs[0]},
		1, &vtype);

	jive_view(graph, stdout);
	assert(floating->parent == region1);

	jive_node * fnode2 = jive_node_create(floating,
		5, (const jive_type *[]){vtype, vtype, vtype, vtype, vtype},
			(jive_output *[]){node0->outputs[0], node1->outputs[0], node2->outputs[0],
			fnode0->outputs[0], fnode1->outputs[0]},
		0, NULL);

	jive_view(graph, stdout);
	assert(floating->parent == region2);

	jive_input_divert_origin(fnode2->inputs[1], node0->outputs[0]);
	assert(floating->parent == region2);	

	jive_input_divert_origin(fnode1->inputs[1], node0->outputs[0]);
	assert(floating->parent == region2);	

	jive_input_divert_origin(fnode2->inputs[2], node0->outputs[0]);

	jive_view(graph, stdout);
	assert(floating->parent == graph->root_region);

	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/test-floating-regions", test_main);
