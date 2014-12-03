/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testtypes.h"

#include <assert.h>
#include <locale.h>

#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/substitution.h>

#include "testnodes.h"

static int test_main(void)
{
	setlocale(LC_ALL, "");
	jive_graph * graph = jive_graph_create();
	
	jive_region * r1 = jive_region_create_subregion(graph->root_region);
	
	jive_test_value_type type;
	jive::ctl::type control_type;
	const jive::base::type * tmparray0[] = {&type, &type, &control_type};
	
	jive_node * top = jive_test_node_create(r1,
		0, NULL, NULL,
		3, tmparray0);
	r1->top = top;
	
	jive::output * tmp;
	const jive::base::type * tmparray1[] = {&type};
	jive_gamma(top->outputs[2],
		1, tmparray1,
		&top->outputs[0], &top->outputs[1], &tmp);
	jive_node * gamma = tmp->node();
	const jive::base::type * tmparray2[] = {&type};
	
	jive_node * bottom = jive_test_node_create(r1,
		1, tmparray2, &gamma->outputs[0],
		0, NULL);
	r1->bottom = bottom;
	
	jive_view(graph, stderr);
	
	jive_region * r2 = jive_region_create_subregion(graph->root_region);
	jive_substitution_map * subst = jive_substitution_map_create();
	jive_region_copy_substitute(r1, r2, subst, true, true);
	jive_substitution_map_destroy(subst);
	
	jive_node * copied_top = r2->top;
	jive_node * copied_bottom = r2->bottom;
	assert(copied_top && copied_top->ninputs == 0 && copied_top->noutputs == 3);
	assert(copied_bottom && copied_bottom->ninputs == 1 && copied_bottom->noutputs == 0);
	jive_node * copied_gamma = copied_bottom->producer(0);
	assert(copied_gamma->operation() == gamma->operation());
	jive_node * alt1 = copied_gamma->producer(0);
	jive_node * alt2 = copied_gamma->producer(1);
	assert(alt1->region->parent == r2);
	assert(alt2->region->parent == r2);
	assert(dynamic_cast<const jive::gamma_tail_op *>(&alt1->operation()));
	assert(dynamic_cast<const jive::gamma_tail_op *>(&alt2->operation()));
	
	jive_view(graph, stderr);
	
	jive_graph_destroy(graph);
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("transforms/test-region-copy", test_main);
