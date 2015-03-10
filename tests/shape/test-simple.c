/*
 * Copyright 2010 2011 2012 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>
#include <stdio.h>

#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/shaped-region.h>
#include <jive/types/bitstring.h>
#include <jive/view.h>
#include <jive/vsdg.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");
	jive_graph * graph = jive_graph_create();
	
	jive::output * y = jive_bitsymbolicconstant(graph, 8, "y");
	jive_output_auto_assign_variable(y);
	
	jive_shaped_graph * shaped_graph = jive_shaped_graph_create(graph);
	
	assert(shaped_graph->map_region(graph->root_region) != NULL);
	assert(shaped_graph->map_ssavar(y->ssavar) != NULL);
	
	jive_region * subregion = jive_region_create_subregion(graph->root_region);
	assert(shaped_graph->map_region(subregion) != NULL);
	
	jive::output * o = jive_bitsymbolicconstant(graph, 8, "x");
	jive_bitslice(o, 0, 4);
	jive_variable * var = jive_variable_create(graph);
	jive_ssavar * ssavar = jive_ssavar_create(o, var);
	
	assert(shaped_graph->map_variable(var) == NULL);
	assert(shaped_graph->map_ssavar(ssavar) == NULL);
	
	jive_ssavar_assign_output(ssavar, o);
	assert(shaped_graph->map_variable(var) != NULL);
	assert(shaped_graph->map_ssavar(ssavar) != NULL);
	
	jive_shaped_region * shaped_root_region = shaped_graph->map_region(
		graph->root_region);
	jive_cut * c1 = shaped_root_region->create_top_cut();
	jive_cut * c2 = shaped_root_region->create_top_cut();
	
	c1->append(y->node());
	c2->append(o->node());
	
	jive_shaped_graph_destroy(shaped_graph);
	
	jive_graph_destroy(graph);
	return 0;
}

JIVE_UNIT_TEST_REGISTER("shape/test-simple", test_main);
