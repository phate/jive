/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testtypes.h"

#include <assert.h>
#include <stdio.h>
#include <locale.h>
#include <jive/vsdg.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/node-private.h>
#include <jive/view.h>

#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/shaped-region.h>
#include <jive/regalloc/shaped-node-private.h>
#include <jive/regalloc/shaped-variable-private.h>
#include <jive/types/bitstring.h>

static void
shape(jive_shaped_graph * shaped_graph, jive_node * node)
{
	jive_shaped_region * shaped_region = jive_shaped_graph_map_region(shaped_graph, node->region);
	jive_cut * cut = jive_shaped_region_create_cut(shaped_region);
	jive_cut_append(cut, node);
}

static size_t
record_iterate(jive_shaped_node_downward_iterator * i, jive_node * dst[], size_t max)
{
	size_t count = 0;
	for(;;) {
		jive_shaped_node * shaped_node;
		shaped_node = jive_shaped_node_downward_iterator_next(i);
		if (!shaped_node) break;
		
		assert(count < max);
		dst[count ++] = shaped_node->node;
	}
	return count;
}

static int test_main(void)
{
	setlocale(LC_ALL, "");
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_test_value_type type;
	jive_anchor_type anchor_type;
	const jive_type * tmparray0[] = {&type};
	
	jive_node * n1 = jive_node_create(graph->root_region,
		0, NULL, NULL,
		1, tmparray0);
	
	jive_region * r1 = jive_region_create_subregion(graph->root_region);
	const jive_type * tmparray1[] = {&type};
	const jive_type * tmparray2[] = {&type};
	
	jive_node * n2 = jive_node_create(r1,
		1, tmparray1, n1->outputs,
		1, tmparray2);
	
	jive_region * r2 = jive_region_create_subregion(r1);
	jive_region * r3 = jive_region_create_subregion(r1);
	const jive_type * tmparray3[] = {&type};
	const jive_type * tmparray4[] = {&anchor_type};
	
	jive_node * n3 = jive_node_create(r2,
		1, tmparray3, n2->outputs,
		1, tmparray4);
	const jive_type * tmparray5[] = {&type};
	const jive_type * tmparray6[] = {&anchor_type};
	jive_node * n4 = jive_node_create(r3,
		1, tmparray5, n2->outputs,
		1, tmparray6);
	const jive_type * tmparray7[] = {&anchor_type, &anchor_type};
	jive_output * tmparray8[] = {n3->outputs[0], n4->outputs[0]};
	const jive_type * tmparray9[] = {&type};
	
	jive_node * n5 = jive_node_create(r1,
		2, tmparray7, tmparray8,
		1, tmparray9);
	const jive_type * tmparray10[] = {&type};
	const jive_type * tmparray11[] = {&anchor_type};
	
	jive_node * n6 = jive_node_create(r1,
		1, tmparray10, n5->outputs,
		1, tmparray11);
	const jive_type * tmparray12[] = {&anchor_type};
	
	jive_node * n7 = jive_node_create(graph->root_region,
		1, tmparray12, n6->outputs,
		0, NULL);
	
	jive_shaped_graph * shaped_graph = jive_shaped_graph_create(graph);
	
	shape(shaped_graph, n7);
	shape(shaped_graph, n6);
	shape(shaped_graph, n5);
	shape(shaped_graph, n4);
	shape(shaped_graph, n3);
	shape(shaped_graph, n2);
	shape(shaped_graph, n1);
	
	jive_shaped_node_downward_iterator i;
	
	size_t count;
	jive_node * nodes[16];
	
	{
		jive_shaped_node_downward_iterator_init(&i, jive_shaped_graph_map_node(shaped_graph, n1));
		count = record_iterate(&i, nodes, 16);
		jive_shaped_node_downward_iterator_fini(&i);
		
		assert(count == 7);
		assert(nodes[0] == n1);
		assert(nodes[1] == n2);
		assert(nodes[2] == n3 || nodes[2] == n4);
		assert(nodes[3] == n4 || nodes[3] == n3);
		assert(nodes[4] == n5);
		assert(nodes[5] == n6);
		assert(nodes[6] == n7);
	}

	{
		jive_shaped_node_downward_iterator_init(&i, jive_shaped_graph_map_node(shaped_graph, n2));
		count = record_iterate(&i, nodes, 16);
		jive_shaped_node_downward_iterator_fini(&i);
		
		assert(count == 5);
		assert(nodes[0] == n2);
		assert(nodes[1] == n3 || nodes[2] == n4);
		assert(nodes[2] == n4 || nodes[3] == n3);
		assert(nodes[3] == n5);
		assert(nodes[4] == n6);
	}
	
	{
		jive_shaped_node_downward_iterator_init(&i, jive_shaped_graph_map_node(shaped_graph, n3));
		count = record_iterate(&i, nodes, 16);
		jive_shaped_node_downward_iterator_fini(&i);
		
		assert(count == 1);
		assert(nodes[0] == n3);
	}
	
	{
		jive_shaped_node_downward_iterator_init_outward(&i, jive_shaped_graph_map_node(shaped_graph, n3));
		count = record_iterate(&i, nodes, 16);
		jive_shaped_node_downward_iterator_fini(&i);
		
		assert(count == 4);
		assert(nodes[0] == n3);
		assert(nodes[1] == n5);
		assert(nodes[2] == n6);
		assert(nodes[3] == n7);
	}
	
	{
		jive_shaped_node_downward_iterator_init(&i, jive_shaped_graph_map_node(shaped_graph, n4));
		count = record_iterate(&i, nodes, 16);
		jive_shaped_node_downward_iterator_fini(&i);
		
		assert(count == 1);
		assert(nodes[0] == n4);
	}
	
	{
		jive_shaped_node_downward_iterator_init_outward(&i, jive_shaped_graph_map_node(shaped_graph, n4));
		count = record_iterate(&i, nodes, 16);
		jive_shaped_node_downward_iterator_fini(&i);
		
		assert(count == 4);
		assert(nodes[0] == n4);
		assert(nodes[1] == n5);
		assert(nodes[2] == n6);
		assert(nodes[3] == n7);
	}
	
	{
		jive_shaped_node_downward_iterator_init(&i, jive_shaped_graph_map_node(shaped_graph, n5));
		count = record_iterate(&i, nodes, 16);
		jive_shaped_node_downward_iterator_fini(&i);
		
		assert(count == 2);
		assert(nodes[0] == n5);
		assert(nodes[1] == n6);
	}
	
	jive_shaped_graph_destroy(shaped_graph);
	
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	return 0;
}

JIVE_UNIT_TEST_REGISTER("shape/test-iteration", test_main);
