/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testnodes.h"
#include "testtypes.h"

#include <assert.h>
#include <stdio.h>
#include <locale.h>
#include <jive/vsdg.h>
#include <jive/view.h>

#include <jive/regalloc/crossing-arc.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/shaped-region.h>
#include <jive/regalloc/shaped-region-private.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/node-private.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);

	jive_test_value_type vtype;
	const jive_type * vtype_ptr = &vtype;
	jive_node * a = jive_test_node_create(graph->root_region, 0, NULL, NULL, 1, &vtype_ptr);
	jive_node * b = jive_test_node_create(graph->root_region, 0, NULL, NULL, 1, &vtype_ptr);
	jive_node * c = jive_test_node_create(graph->root_region, 1, &vtype_ptr, &a->outputs[0], 1,
		&vtype_ptr);
	jive_region * subregion = jive_region_create_subregion(graph->root_region);
	jive_node * d = jive_test_node_create(subregion, 1, &vtype_ptr, &a->outputs[0], 1, &vtype_ptr);
	jive_node * e = jive_test_node_create(graph->root_region, 1, &vtype_ptr, &b->outputs[0], 1,
		&vtype_ptr);
	jive_node * f = jive_test_node_create(graph->root_region, 1, &vtype_ptr, &b->outputs[0], 1,
		&vtype_ptr);

	jive::achr::type anchor_type;
	jive_node_add_input(e, &anchor_type, jive_node_add_output(d, &anchor_type));
	
	jive_view(graph, stdout);
	
	jive_shaped_graph * shaped_graph = jive_shaped_graph_create(graph);
	
	jive_shaped_region * root = jive_shaped_graph_map_region(shaped_graph, graph->root_region);
	jive_shaped_region * sub = jive_shaped_graph_map_region(shaped_graph, subregion);
	
	jive_node_auto_merge_variables(a);
	jive_node_auto_merge_variables(b);
	jive_node_auto_merge_variables(c);
	jive_node_auto_merge_variables(d);
	jive_node_auto_merge_variables(e);
	
	jive_crossing_arc_iterator i;
	
	jive_shaped_node * fp = jive_cut_append(jive_shaped_region_create_cut(root), f);
	jive_shaped_node * ep = jive_cut_append(jive_shaped_region_create_cut(root), e);
	jive_shaped_node * dp = jive_cut_append(jive_shaped_region_create_cut(sub), d);
	jive_shaped_node * cp = jive_cut_append(jive_shaped_region_create_cut(root), c);
	jive_shaped_node * bp = jive_cut_append(jive_shaped_region_create_cut(root), b);
	
	jive_shaped_ssavar_lower_boundary_region_depth(jive_shaped_graph_map_ssavar(shaped_graph, a->outputs[0]->ssavar), 1);
	jive_crossing_arc_iterator_init_ssavar(&i, NULL, dp, jive_shaped_graph_map_ssavar(shaped_graph, a->outputs[0]->ssavar));
	assert(i.region == sub && i.node == NULL);
	jive_crossing_arc_iterator_next(&i);
	assert(i.region == NULL && i.node == NULL);
	
	jive_shaped_node * ap = jive_cut_append(jive_shaped_region_create_cut(root), a);
	
	jive_crossing_arc_iterator_init_ssavar(&i, ap, dp, jive_shaped_graph_map_ssavar(shaped_graph, a->outputs[0]->ssavar));
	assert(i.region == sub && i.node == NULL);
	jive_crossing_arc_iterator_next(&i);
	assert(i.region == root && i.node == cp);
	
	assert(jive_shaped_node_prev_in_region(ep) == cp);
	assert(jive_shaped_node_prev_in_region(dp) == NULL);
	assert(jive_shaped_node_prev_in_region(cp) == bp);
	assert(jive_shaped_node_prev_in_region(bp) == ap);
	assert(jive_shaped_node_prev_in_region(ap) == NULL);
	
	jive_crossing_arc_iterator_init_ssavar(&i, ap, cp, jive_shaped_graph_map_ssavar(shaped_graph, a->outputs[0]->ssavar));
	assert(i.region == root && i.node == bp);
	jive_crossing_arc_iterator_next(&i);
	assert(i.region == NULL && i.node == NULL);
	
	jive_crossing_arc_iterator_init_ssavar(&i, bp, fp, jive_shaped_graph_map_ssavar(shaped_graph, b->outputs[0]->ssavar));
	assert(i.region == root && i.node == ep);
	jive_crossing_arc_iterator_next(&i);
	assert(i.region == sub && i.node == dp);
	jive_crossing_arc_iterator_next(&i);
	assert(i.region == sub && i.node == NULL);
	jive_crossing_arc_iterator_next(&i);
	assert(i.region == root && i.node == cp);
	jive_crossing_arc_iterator_next(&i);
	assert(i.region == NULL && i.node == NULL);
	
	/* for testing purposes just "fake" merged setting */
	jive_shaped_ssavar_set_boundary_region_depth(jive_shaped_graph_map_ssavar(shaped_graph, a->outputs[0]->ssavar), 0);
	jive_crossing_arc_iterator_init_ssavar(&i, ap, dp, jive_shaped_graph_map_ssavar(shaped_graph, a->outputs[0]->ssavar));
	assert(i.region == sub && i.node == NULL);
	jive_crossing_arc_iterator_next(&i);
	assert(i.region == root && i.node == cp);
	jive_crossing_arc_iterator_next(&i);
	assert(i.region == root && i.node == bp);
	jive_crossing_arc_iterator_next(&i);
	assert(i.region == NULL && i.node == NULL);
	/* don't forget to reset */
	jive_shaped_ssavar_set_boundary_region_depth(jive_shaped_graph_map_ssavar(shaped_graph, a->outputs[0]->ssavar), 1);
	
	assert(sub->active_top.base.ssavar_map.nitems == 2);
	jive_cutvar_xpoint * xpoint;
	xpoint = jive_cutvar_xpoint_hash_byssavar_lookup(&sub->active_top.base.ssavar_map, jive_shaped_graph_map_ssavar(shaped_graph, a->outputs[0]->ssavar));
	assert(xpoint);
	assert(xpoint->count == 1);
	xpoint = jive_cutvar_xpoint_hash_byssavar_lookup(&sub->active_top.base.ssavar_map, jive_shaped_graph_map_ssavar(shaped_graph, b->outputs[0]->ssavar));
	assert(xpoint);
	assert(xpoint->count == 1);
	
	jive_shaped_graph_destroy(shaped_graph);
	
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	return 0;
}

JIVE_UNIT_TEST_REGISTER("shape/test-crossing-arc", test_main);
