/*
 * Copyright 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testtypes.h"

#include <locale.h>
#include <assert.h>

#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/theta.h>
#include <jive/vsdg/node-private.h>

#include "testnodes.h"

static int test_main(void)
{
	setlocale(LC_ALL, "");
	
	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);
	
	jive_test_value_type type;
	jive_control_type ctype;
	const jive_type * ctype_ptr = &ctype;
	const jive_type * tmparray0[] = {&type};
	
	jive_node * top = jive_test_node_create(graph->root_region,
		0, NULL, NULL,
		1, tmparray0);
	
	jive_theta theta1 = jive_theta_begin(graph);
	jive_theta_loopvar loopvar1 = jive_theta_loopvar_enter(theta1, top->outputs[0]);
	assert(theta1.region->parent == graph->root_region);
	
	jive_theta theta2 = jive_theta_begin(graph);
	jive_theta_loopvar loopvar2 = jive_theta_loopvar_enter(theta2, top->outputs[0]);
	assert(theta2.region->parent == graph->root_region);
	
	jive_theta theta3 = jive_theta_begin(graph);
	jive_theta_loopvar loopvar3 = jive_theta_loopvar_enter(theta3, loopvar2.value);
	assert(theta3.region->parent == theta2.region);
	
	jive_view(graph, stdout);
	/* the only way to satisfy the following edge is to move both theta2
	and the (nested) theta3 inside theta1 */
	jive_theta_loopvar loopvar4 = jive_theta_loopvar_enter(theta3, loopvar1.value);
	jive_view(graph, stdout);
	fprintf(stderr, "done loopvar4\n");
	(void)loopvar4;
	assert(theta3.region->parent == theta2.region);
	assert(theta2.region->parent == theta1.region);
	assert(theta1.region->parent == graph->root_region);
	
	jive_node * pred3 = jive_test_node_create(theta3.region,
		1, tmparray0, &loopvar3.value,
		1, &ctype_ptr);
	jive_view(graph, stdout);
	jive_theta_end(theta3, pred3->outputs[0], 1, &loopvar3);
	
	jive_view(graph, stdout);
	jive_node * pred2 = jive_test_node_create(theta2.region,
		1, tmparray0, &loopvar3.value,
		1, &ctype_ptr);
	jive_view(graph, stdout);
	jive_theta_end(theta2, pred2->outputs[0], 1, &loopvar2);
	
	jive_node * pred1 = jive_test_node_create(theta1.region,
		1, tmparray0, &loopvar2.value,
		1, &ctype_ptr);
	jive_theta_end(theta1, pred1->outputs[0], 1, &loopvar1);
	
	jive_view(graph, stdout);
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/test-nested-floating-theta", test_main);
