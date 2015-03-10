/*
 * Copyright 2010 2011 2012 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testnodes.h"
#include "testtypes.h"

#include <assert.h>
#include <locale.h>
#include <stdio.h>

// FIXME: oh the horror...
#define private public
#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/shaped-node.h>
#include <jive/regalloc/shaped-region.h>
#include <jive/regalloc/shaped-variable.h>
#include <jive/view.h>
#include <jive/vsdg.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");
	jive_graph * graph = jive_graph_create();
	
	jive_test_value_type vtype;
	jive::output * x = jive_test_node_create(graph->root_region, {}, {}, {&vtype})->outputs[0];
	jive::output * y = jive_test_node_create(graph->root_region, {}, {}, {&vtype})->outputs[0];
	jive_node * n = jive_test_node_create(graph->root_region, {&vtype, &vtype}, {x, y}, {&vtype});

	jive_output_auto_assign_variable(x);
	jive_output_auto_assign_variable(y);
	
	jive_shaped_graph * shaped_graph = jive_shaped_graph_create(graph);
	
	jive_shaped_region * r = shaped_graph->map_region(graph->root_region);
	jive_cut * lower = r->create_top_cut();
	jive_cut * upper = r->create_top_cut();
	
	jive_shaped_node * nn = lower->append(n);
	jive_shaped_node * nx = upper->append(x->node());
	jive_shaped_node * ny = upper->append(y->node());
	
	jive_shaped_variable * vx = shaped_graph->map_variable(x->ssavar->variable);
	jive_shaped_variable * vy = shaped_graph->map_variable(y->ssavar->variable);
	
	jive_shaped_ssavar * sx = shaped_graph->map_ssavar(x->ssavar);
	jive_shaped_ssavar * sy = shaped_graph->map_ssavar(y->ssavar);
	
	nx->add_ssavar_after(sx, vx->variable(), 1);
	ny->add_ssavar_after(sy, vy->variable(), 1);
	
	assert(vx->interferes_with(vy) == 0);
	
	nn->add_ssavar_before(sx, vx->variable(), 1);
	nn->add_ssavar_before(sy, vy->variable(), 1);
	
	assert(vx->interferes_with(vy) == 1);
	
	nx->remove_ssavar_after(sx, vx->variable(), 1);
	ny->remove_ssavar_after(sy, vy->variable(), 1);
	nn->remove_ssavar_before(sx, vx->variable(), 1);
	nn->remove_ssavar_before(sy, vy->variable(), 1);
	
	jive_shaped_graph_destroy(shaped_graph);
	
	jive_graph_destroy(graph);
	return 0;
}

JIVE_UNIT_TEST_REGISTER("shape/test-interference", test_main);
