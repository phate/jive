/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
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
	jive_context * ctx = jive_context_create();
	
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_region * r1 = jive_region_create_subregion(graph->root_region);
	
	jive_test_value_type type;
	jive_control_type control_type;
	const jive_type * tmparray0[] = {&type, &type, &control_type};
	
	jive_node * top = jive_test_node_create(r1,
		0, NULL, NULL,
		3, tmparray0);
	r1->top = top;
	
	jive_output * tmp;
	const jive_type * tmparray1[] = {&type};
	jive_gamma(top->outputs[2],
		1, tmparray1,
		&top->outputs[0], &top->outputs[1], &tmp);
	jive_node * gamma = tmp->node;
	const jive_type * tmparray2[] = {&type};
	
	jive_node * bottom = jive_test_node_create(r1,
		1, tmparray2, &gamma->outputs[0],
		0, NULL);
	r1->bottom = bottom;
	
	jive_view(graph, stderr);
	
	jive_region * r2 = jive_region_create_subregion(graph->root_region);
	jive_substitution_map * subst = jive_substitution_map_create(ctx);
	jive_region_copy_substitute(r1, r2, subst, true, true);
	jive_substitution_map_destroy(subst);
	
	jive_node * copied_top = r2->top;
	jive_node * copied_bottom = r2->bottom;
	assert(copied_top && copied_top->ninputs == 0 && copied_top->noutputs == 3);
	assert(copied_bottom && copied_bottom->ninputs == 1 && copied_bottom->noutputs == 0);
	jive_node * copied_gamma = copied_bottom->inputs[0]->origin()->node;
	assert(copied_gamma->class_ == gamma->class_);
	jive_node * alt1 = copied_gamma->inputs[0]->origin()->node;
	jive_node * alt2 = copied_gamma->inputs[1]->origin()->node;
	assert(alt1->region->parent == r2);
	assert(alt2->region->parent == r2);
	assert(alt1->class_ = &JIVE_GAMMA_TAIL_NODE);
	assert(alt2->class_ = &JIVE_GAMMA_TAIL_NODE);
	
	jive_view(graph, stderr);
	
	jive_graph_destroy(graph);
	
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	return 0;
}

JIVE_UNIT_TEST_REGISTER("transforms/test-region-copy", test_main);
