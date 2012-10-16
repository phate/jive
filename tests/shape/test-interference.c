/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <stdio.h>
#include <locale.h>
#include <jive/vsdg.h>
#include <jive/view.h>

#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/shaped-region.h>
#include <jive/regalloc/shaped-node-private.h>
#include <jive/regalloc/shaped-variable-private.h>
#include <jive/types/bitstring.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_output * x = jive_bitsymbolicconstant(graph, 8, "x");
	jive_output * y = jive_bitsymbolicconstant(graph, 8, "y");
	jive_node * n = jive_bitconcat_create(graph->root_region, 2, (jive_output *[]){x, y});
	jive_output_auto_assign_variable(x);
	jive_output_auto_assign_variable(y);
	
	jive_shaped_graph * shaped_graph = jive_shaped_graph_create(graph);
	
	jive_shaped_region * r = jive_shaped_graph_map_region(shaped_graph, graph->root_region);
	jive_cut * lower = jive_shaped_region_create_cut(r);
	jive_cut * upper = jive_shaped_region_create_cut(r);
	
	jive_shaped_node * nn = jive_cut_append(lower, n);
	jive_shaped_node * nx = jive_cut_append(upper, x->node);
	jive_shaped_node * ny = jive_cut_append(upper, y->node);
	
	jive_shaped_variable * vx = jive_shaped_graph_map_variable(shaped_graph, x->ssavar->variable);
	jive_shaped_variable * vy = jive_shaped_graph_map_variable(shaped_graph, y->ssavar->variable);
	
	jive_shaped_ssavar * sx = jive_shaped_graph_map_ssavar(shaped_graph, x->ssavar);
	jive_shaped_ssavar * sy = jive_shaped_graph_map_ssavar(shaped_graph, y->ssavar);
	
	jive_shaped_node_add_ssavar_after(nx, sx, vx->variable, 1);
	jive_shaped_node_add_ssavar_after(ny, sy, vy->variable, 1);
	
	assert(jive_shaped_variable_interferes_with(vx, vy) == 0);
	
	jive_shaped_node_add_ssavar_before(nn, sx, vx->variable, 1);
	jive_shaped_node_add_ssavar_before(nn, sy, vy->variable, 1);
	
	assert(jive_shaped_variable_interferes_with(vx, vy) == 1);
	
	jive_shaped_node_remove_ssavar_after(nx, sx, vx->variable, 1);
	jive_shaped_node_remove_ssavar_after(ny, sy, vy->variable, 1);
	jive_shaped_node_remove_ssavar_before(nn, sx, vx->variable, 1);
	jive_shaped_node_remove_ssavar_before(nn, sy, vy->variable, 1);
	
	jive_shaped_graph_destroy(shaped_graph);
	
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	return 0;
}

JIVE_UNIT_TEST_REGISTER("shape/test-interference", test_main);
