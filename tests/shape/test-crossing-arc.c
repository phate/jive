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

#include <jive/regalloc/crossing-arc.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/shaped-region.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/node-private.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");
	jive_graph * graph = jive_graph_create();

	jive_test_value_type vtype;
	jive_node * a = jive_test_node_create(graph->root_region, {}, {}, {&vtype});
	jive_node * b = jive_test_node_create(graph->root_region, {}, {}, {&vtype});
	jive_node * c = jive_test_node_create(graph->root_region, {&vtype}, {a->outputs[0]}, {&vtype});

	jive_region * subregion = jive_region_create_subregion(graph->root_region);
	jive_node * d = jive_test_node_create(subregion, {&vtype}, {a->outputs[0]}, {&vtype});
	jive_node * e = jive_test_node_create(graph->root_region, {&vtype}, {b->outputs[0]}, {&vtype});
	jive_node * f = jive_test_node_create(graph->root_region, {&vtype}, {b->outputs[0]}, {&vtype});

	jive::achr::type anchor_type;
	jive_node_add_input(e, &anchor_type, jive_node_add_output(d, &anchor_type));
	
	jive_view(graph, stdout);
	
	jive_shaped_graph * shaped_graph = jive_shaped_graph_create(graph);
	
	jive_shaped_region * root = shaped_graph->map_region(graph->root_region);
	jive_shaped_region * sub = shaped_graph->map_region(subregion);
	
	jive_node_auto_merge_variables(a);
	jive_node_auto_merge_variables(b);
	jive_node_auto_merge_variables(c);
	jive_node_auto_merge_variables(d);
	jive_node_auto_merge_variables(e);
	
	jive_crossing_arc_iterator i;
	
	jive_shaped_node * fp = root->create_top_cut()->append(f);
	jive_shaped_node * ep = root->create_top_cut()->append(e);
	jive_shaped_node * dp = sub->create_top_cut()->append(d);
	jive_shaped_node * cp = root->create_top_cut()->append(c);
	jive_shaped_node * bp = root->create_top_cut()->append(b);
	
	shaped_graph->map_ssavar(a->outputs[0]->ssavar)->lower_boundary_region_depth(1);
	jive_crossing_arc_iterator_init_ssavar(&i, NULL, dp, shaped_graph->map_ssavar(
		a->outputs[0]->ssavar));
	assert(i.region == sub && i.node == NULL);
	jive_crossing_arc_iterator_next(&i);
	assert(i.region == NULL && i.node == NULL);
	
	jive_shaped_node * ap = root->create_top_cut()->append(a);
	
	jive_crossing_arc_iterator_init_ssavar(&i, ap, dp, shaped_graph->map_ssavar(
		a->outputs[0]->ssavar));
	assert(i.region == sub && i.node == NULL);
	jive_crossing_arc_iterator_next(&i);
	assert(i.region == root && i.node == cp);
	
	assert(ep->prev_in_region() == cp);
	assert(dp->prev_in_region() == NULL);
	assert(cp->prev_in_region() == bp);
	assert(bp->prev_in_region() == ap);
	assert(ap->prev_in_region() == nullptr);
	
	jive_crossing_arc_iterator_init_ssavar(&i, ap, cp, shaped_graph->map_ssavar(
		a->outputs[0]->ssavar));
	assert(i.region == root && i.node == bp);
	jive_crossing_arc_iterator_next(&i);
	assert(i.region == NULL && i.node == NULL);
	
	jive_crossing_arc_iterator_init_ssavar(&i, bp, fp, shaped_graph->map_ssavar(
		b->outputs[0]->ssavar));
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
	shaped_graph->map_ssavar(a->outputs[0]->ssavar)->set_boundary_region_depth(0);
	jive_crossing_arc_iterator_init_ssavar(&i, ap, dp, shaped_graph->map_ssavar(
		a->outputs[0]->ssavar));
	assert(i.region == sub && i.node == NULL);
	jive_crossing_arc_iterator_next(&i);
	assert(i.region == root && i.node == cp);
	jive_crossing_arc_iterator_next(&i);
	assert(i.region == root && i.node == bp);
	jive_crossing_arc_iterator_next(&i);
	assert(i.region == NULL && i.node == NULL);
	/* don't forget to reset */
	shaped_graph->map_ssavar(a->outputs[0]->ssavar)->set_boundary_region_depth(1);
	
	assert(sub->active_top().ssavar_map().size() == 2);
	auto xi = sub->active_top().ssavar_map().find(shaped_graph->map_ssavar(
		a->outputs[0]->ssavar));
	assert(xi != sub->active_top().ssavar_map().end());
	assert(xi->count() == 1);
	xi = sub->active_top().ssavar_map().find(shaped_graph->map_ssavar(
		b->outputs[0]->ssavar));
	assert(xi != sub->active_top().ssavar_map().end());
	assert(xi->count() == 1);
	
	jive_shaped_graph_destroy(shaped_graph);
	
	jive_graph_destroy(graph);
	return 0;
}

JIVE_UNIT_TEST_REGISTER("shape/test-crossing-arc", test_main);
