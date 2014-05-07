/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testtypes.h"

#include <assert.h>
#include <stdio.h>
#include <locale.h>
#include <jive/vsdg.h>
#include <jive/view.h>

#include <jive/vsdg/node-private.h>

typedef struct graph_desc {
	jive_graph * graph;
	jive_node * a1, * a2;
	jive_node * b1, * b2;
} graph_desc;

static graph_desc
prepare_graph(jive_context * ctx)
{
	graph_desc g;
	g.graph = jive_graph_create(ctx);
	
	jive_region * region = g.graph->root_region;
	JIVE_DECLARE_TEST_VALUE_TYPE(type);
	const jive_type * tmparray0[] = {type};
	
	g.a1 = jive_node_create(region,
		0, NULL, NULL,
		1, tmparray0);
	const jive_type * tmparray1[] = {type};
	const jive_type * tmparray2[] = {type};
	
	g.a2 = jive_node_create(region,
		1, tmparray1, &g.a1->outputs[0],
		0, tmparray2);
	const jive_type * tmparray3[] = {type};
	
	g.b1 = jive_node_create(region,
		0, NULL, NULL,
		1, tmparray3);
	const jive_type * tmparray4[] = {type};
	const jive_type * tmparray5[] = {type};
	
	g.b2 = jive_node_create(region,
		1, tmparray4, &g.b1->outputs[0],
		0, tmparray5);
	
	return g;
}

static void
test_simple_upward_cone(jive_context * ctx)
{
	graph_desc g = prepare_graph(ctx);
	
	jive_traverser * trav = jive_upward_cone_traverser_create(g.a2);
	
	assert( jive_traverser_next(trav) == g.a2 );
	assert( jive_traverser_next(trav) == g.a1 );
	assert( jive_traverser_next(trav) == NULL );
	
	jive_traverser_destroy(trav);
	
	jive_graph_destroy(g.graph);
}

static void
test_mutable_upward_cone_1(jive_context * ctx)
{
	graph_desc g = prepare_graph(ctx);
	
	jive_traverser * trav = jive_upward_cone_traverser_create(g.a2);
	
	assert( jive_traverser_next(trav) == g.a2 );
	jive_node_destroy(g.b2);
	assert( jive_traverser_next(trav) == g.a1 );
	assert( jive_traverser_next(trav) == NULL );
	
	jive_traverser_destroy(trav);
	
	jive_graph_destroy(g.graph);
}

static void
test_mutable_upward_cone_2(jive_context * ctx)
{
	graph_desc g = prepare_graph(ctx);
	
	jive_traverser * trav = jive_upward_cone_traverser_create(g.a2);
	
	jive_node_destroy(g.a2);
	assert( jive_traverser_next(trav) == g.a1 );
	assert( jive_traverser_next(trav) == NULL );
	
	jive_traverser_destroy(trav);
	
	jive_graph_destroy(g.graph);
}

static void
test_mutable_upward_cone_3(jive_context * ctx)
{
	graph_desc g = prepare_graph(ctx);
	
	jive_traverser * trav = jive_upward_cone_traverser_create(g.a2);
	
	g.a2->inputs[0]->divert_origin(g.b1->outputs[0]);
	assert( jive_traverser_next(trav) == g.a2 );
	assert( jive_traverser_next(trav) == g.b1 );
	
	jive_traverser_destroy(trav);
	
	jive_graph_destroy(g.graph);
}

static int test_main(void)
{
	jive_context * ctx = jive_context_create();
	
	test_simple_upward_cone(ctx);
	test_mutable_upward_cone_1(ctx);
	test_mutable_upward_cone_2(ctx);
	test_mutable_upward_cone_3(ctx);
	
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/test-cone-traverser", test_main);
