/*
 * Copyright 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
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
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive_test_value_type vtype;
	const jive::base::type * vtype_ptr = &vtype;
	jive_region * region0 = graph->root_region;
	jive_node * node0 = jive_test_node_create(region0,
		0, NULL, NULL,
		1, &vtype_ptr);

	jive_region * region1 = jive_region_create_subregion(graph->root_region);
	jive_node * node1 = jive_test_node_create(region1,
		0, NULL, NULL,
		1, &vtype_ptr);

	jive_region * region2 = jive_region_create_subregion(region1);
	jive_node * node2 = jive_test_node_create(region2,
		0, NULL, NULL,
		1, &vtype_ptr);


	jive_floating_region floating = jive_floating_region_create(graph);
	const jive::base::type * tmparray0[] = {&vtype};
	jive::output * tmparray1[] = {node0->outputs[0]};
	jive_node * fnode0 = jive_test_node_create(floating.region,
		1, tmparray0,
			tmparray1,
		1, &vtype_ptr);

	jive_view(graph, stdout);
	assert(floating.region->parent == graph->root_region);
	const jive::base::type * tmparray2[] = {&vtype, &vtype, &vtype};
	jive::output * tmparray3[] = {node0->outputs[0], node1->outputs[0], fnode0->outputs[0]};

	jive_node * fnode1 = jive_test_node_create(floating.region,
		3, tmparray2,
			tmparray3,
		1, &vtype_ptr);

	jive_view(graph, stdout);
	assert(floating.region->parent == region1);
	const jive::base::type * tmparray4[] = {&vtype, &vtype, &vtype, &vtype, &vtype};
	jive::output * tmparray5[] = {node0->outputs[0], node1->outputs[0], node2->outputs[0],
			fnode0->outputs[0], fnode1->outputs[0]};

	jive_node * fnode2 = jive_test_node_create(floating.region,
		5, tmparray4,
			tmparray5,
		0, NULL);

	jive_view(graph, stdout);
	assert(floating.region->parent == region2);

	fnode2->inputs[1]->divert_origin(node0->outputs[0]);
	assert(floating.region->parent == region2);

	fnode1->inputs[1]->divert_origin(node0->outputs[0]);
	assert(floating.region->parent == region2);

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/test-floating-regions", test_main);
